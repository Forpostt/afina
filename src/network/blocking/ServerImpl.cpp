#include "ServerImpl.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <algorithm>

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
        close(item->second);
    } catch (std::runtime_error &ex) {
        std::cerr << "Connection fails: " << ex.what() << std::endl;
        close(item->second);
    }
    std::cout << "End of work client" << std::endl;
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

    max_workers = n_workers;
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
    //pthread_join(accept_thread, NULL);    it's can be already done
    std::unique_lock<std::mutex> lock(connections_mutex);
    while (!connections.empty()) {
        connections_cv.wait(lock);
    }
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

        if (connections.size() >= max_workers) {
            close(client_socket);
            continue;
        }

        pthread_t worker;
        auto args = std::make_pair(this, client_socket);

        if (pthread_create(&worker, nullptr, RunConnectionProxy, &args) != 0) {
            close(client_socket);
        }

        pthread_detach(worker);

        connections.insert(worker);
    }
    // Cleanup on exit..
    close(server_socket);
}

// See Server.h
void ServerImpl::RunConnection(int client_socket) {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;

    Afina::Protocol::Parser parser;
    const size_t buff_size = 1 << 12;
    char buff[buff_size];
    size_t size = 0, parsed;
    ssize_t n;
    uint32_t args_size;

    while (running.load()) {
        if ((n = recv(client_socket, buff, buff_size, 0)) <= 0) {
            break;
        }
        size = n;
        std::cout << buff << std::endl;
        try {
            while (parser.Parse(buff, size, parsed)) {
                auto command = parser.Build(args_size);
                parser.Reset();
                if (args_size > 0) {
                    args_size += 2; // "\r\n"
                }
                while (size - parsed < args_size) {
                    if (size == buff_size) {
                        throw std::invalid_argument("The argument string is too long");
                    }
                    if ((n = recv(client_socket, &buff[size], buff_size - size, 0)) <= 0) {
                        throw std::logic_error("Client off");
                    }
                    size += n;
                }
                std::string out, arg(&buff[parsed], std::max(0, (int) args_size - 2));
                command->Execute(*pStorage, arg, out);
                out += "\r\n";

                if (send(client_socket, out.data(), out.size(), 0) <= 0) {
                    throw std::runtime_error("Socket send() failed");
                }
                if (size > parsed + args_size) {
                    std::memmove(&buff[0], &buff[parsed + args_size], size - parsed - args_size);
                }
                size -= parsed + args_size;
            }
        } catch (std::invalid_argument &e){
            std::string out("SERVER_ERROR: ");
            out += e.what();
            out += "\r\n";
            if (send(client_socket, out.data(), out.size(), 0) <= 0) {
                throw std::runtime_error("Socket send() failed");
            }
        } catch (std::logic_error &e){
            std::cout << e.what() << std::endl;
            break;
        }
    }
    //usleep(6000000);
    std::lock_guard<std::mutex> lock(connections_mutex);
    connections.erase(pthread_self());
    connections_cv.notify_one();
}


} // namespace Blocking
} // namespace Network
} // namespace Afina
