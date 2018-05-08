#include "ServerImpl.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

#include <pthread.h>
#include <signal.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <afina/Storage.h>
#include <protocol/Parser.h>

namespace Afina {
namespace Network {
namespace Blocking {

void *ServerImpl::RunAcceptorProxy(void *p) {
    ServerImpl *srv = reinterpret_cast<ServerImpl *>(p);
    try {
        srv->RunAcceptor();
    } catch (std::runtime_error &ex) {
        std::cerr << "Server fails: " << ex.what() << std::endl;
    }

    return 0;
}

void *ServerImpl::RunConnectionProxy(void *p) {
    auto *item = reinterpret_cast<std::pair<ServerImpl *, int> *>(p);
    try {
        item->first->RunConnection(item->second);
    } catch (std::runtime_error &ex) {
        std::cerr << "Connection fails: " << ex.what() << std::endl;
        close(item->second);
    }
    std::cout << "End of work client" << std::endl;
    {
        std::lock_guard<std::mutex> lock(item->first->connections_mutex);
        item->first->connections.erase(pthread_self());
    }
    return 0;
}


// See Server.h
ServerImpl::ServerImpl(std::shared_ptr<Afina::Storage> ps) : Server(ps) {}

// See Server.h
ServerImpl::~ServerImpl() {}

// See Server.h
void ServerImpl::Start(uint32_t port, uint16_t n_workers) {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;

    // If a client closes a connection, this will generally produce a SIGPIPE
    // signal that will kill the process. We want to ignore this signal, so send()
    // just returns -1 when this happens.
    sigset_t sig_mask;
    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SIGPIPE);
    if (pthread_sigmask(SIG_BLOCK, &sig_mask, nullptr) != 0) {
        throw std::runtime_error("Unable to mask SIGPIPE");
    }

    //max_workers = n_workers;
    max_workers = 5;
    listen_port = port;

    running.store(true);
    if (pthread_create(&accept_thread, nullptr, ServerImpl::RunAcceptorProxy, this) < 0) {
        throw std::runtime_error("Could not create server thread");
    }
}

// See Server.h
void ServerImpl::Stop() {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;

    running.store(false);

    Join();
}

// See Server.h
void ServerImpl::Join() {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;

    //TODO: Is it necessary
    std::unique_lock<std::mutex> lock(connections_mutex);

    pthread_join(accept_thread, 0);
    for (auto&& thread : connections)
        pthread_join(thread, 0);

    connections.clear();
}

// See Server.h
void ServerImpl::RunAcceptor() {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;

    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;          // IPv4
    server_addr.sin_port = htons(listen_port); // TCP port number
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Bind to any address


    int server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1) {
        throw std::runtime_error("Failed to open socket");
    }

    // when the server closes the socket,the connection must stay in the TIME_WAIT state to
    // make sure the client received the acknowledgement that the connection has been terminated.
    // During this time, this port is unavailable to other processes, unless we specify this option
    //
    // This option let kernel knows that we are OK that multiple threads/processes are listen on the
    // same port. In a such case kernel will balance input traffic between all listeners (except those who
    // are closed already)
    int opts = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts)) == -1) {
        close(server_socket);
        throw std::runtime_error("Socket setsockopt() failed");
    }

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        close(server_socket);
        throw std::runtime_error("Socket bind() failed");
    }

    // Start listening. The second parameter is the "backlog", or the maximum number of
    // connections that we'll allow to queue up. Note that listen() doesn't block until
    // incoming connections arrive. It just makesthe OS aware that this process is willing
    // to accept connections on this socket (which is bound to a specific IP and port)
    if (listen(server_socket, 5) == -1) {
        close(server_socket);
        throw std::runtime_error("Socket listen() failed");
    }

    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t sinSize = sizeof(struct sockaddr_in);
    while (running.load()) {
        std::cout << "network debug: waiting for connection..." << std::endl;

        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &sinSize)) == -1) {
            close(server_socket);
            throw std::runtime_error("Socket accept() failed");
        }

        std::lock_guard<std::mutex> lock(connections_mutex);

        if (connections.size() >= max_workers){
            std::string msg = "Too much connections, try latter\r\n";

            if (send(client_socket, msg.data(), msg.size(), 0) <= 0) {
                close(client_socket);
                close(server_socket);
                throw std::runtime_error("Socket send() failed");
            }
            close(client_socket);
        } else {
            pthread_t worker;
            auto args = std::make_pair(this, client_socket);

            if (pthread_create(&worker, nullptr, ServerImpl::RunConnectionProxy, &args) < 0) {
                close(client_socket);
                close(server_socket);
                throw std::runtime_error("Thread create() error in acceptor");
            }
            connections.insert(worker);
        }
    }
    // Cleanup on exit..
    close(server_socket);
}

// See Server.h
void ServerImpl::RunConnection(int client_socket) {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;
    Afina::Protocol::Parser parser;

    const size_t buff_size = 1024;
    char buff[buff_size];
    std::string cur_data;
    size_t parsed = 0;
    ssize_t n;
    memset(buff, 0, buff_size);

    while ((n = recv(client_socket, buff, buff_size, 0)) > 0) {
        cur_data.append(buff, n);
        memset(buff, 0, buff_size);

        while (parser.Parse(&cur_data[parsed], cur_data.size() - parsed, parsed)) {
            uint32_t args_size;
            auto command = parser.Build(args_size);

            if (args_size != 0){
                size_t rest = parsed + args_size + msg_end.size() - cur_data.size();
                if (rest > 0) {
                    if (recv(client_socket, buff, rest, MSG_WAITALL) < 0) {
                        throw std::runtime_error("Error");
                    }
                    cur_data.append(buff, rest);
                    memset(buff, 0, buff_size);
                }
            }

            /* TODO: Change to std::exception, 'get' doesn't have value
            if (cur_data is not valid) {
                std::string msg = "Wrong value bytes\r\n";
                if (send(client_socket, msg.data(), msg.size(), 0) <= 0){
                    close(client_socket);
                    throw std::runtime_error("Error");
                }
            }
            */

            std::string out;
            command->Execute(*pStorage, cur_data.substr(parsed, args_size), out);
            if (send(client_socket, (out + msg_end).data(), out.size() + msg_end.size(), 0) <= 0){
                throw std::runtime_error("Error");
            }
            std::cout << out << std::endl;
            parser.Reset();
            if (args_size > 0) {
                cur_data = cur_data.substr(parsed + args_size + msg_end.size());
            } else {
                cur_data.clear();
            }
            parsed = 0;

            if (!running.load()) {
                std::cout << "!running.load()" << std::endl;
                return;
            }
        }
        if (cur_data.size() > buff_size * 2) {
            std::string msg = "Too much symbols\r\n";
            if (send(client_socket, msg.data(), msg.size(), 0) <= 0){
                throw std::runtime_error("Error");
            }
        }
    }
    //usleep(60000000);
    if (n < 0){
        throw std::runtime_error("Error");
    }
    close(client_socket);
}


} // namespace Blocking
} // namespace Network
} // namespace Afina
