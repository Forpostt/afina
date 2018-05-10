#include "Worker.h"

#include <iostream>
#include <stdexcept>
#include <utility>
#include <cassert>
#include <cstring>

#include <csignal>

#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#include "Utils.h"
#include "Connection.h"

#define MAXEVENTS 64
#define MAXCONN 64
#define EPOLLEXCLUSIVE 1 << 28

namespace Afina {
namespace Network {
namespace NonBlocking {

// See Worker.h
Worker::Worker(std::shared_ptr<Afina::Storage> ps): pStorage(ps) {}

// See Worker.h
Worker::~Worker() {
    //TODO: check here
    Stop();
    Join();
}

void* Worker::OnRunProxy(void *p) {
    Worker *item = reinterpret_cast<Worker *>(p);
    try {
        int t = item->server_fd.load();
        item->OnRun(t);
    } catch (std::runtime_error &ex) {
        std::cerr << "Worker fails: " << ex.what() << std::endl;
        //close(item->second);
    }
    std::cout << "End of worker" << std::endl;
    return 0;
}


// See Worker.h
void Worker::Start(int server_socket) {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;

    running.store(true);
    //TODO: Why we should use std::atomic here?
    server_fd.store(server_socket);

    if (pthread_create(&thread, nullptr, Worker::OnRunProxy, this) < 0) {
        throw std::runtime_error("Could not create server thread");
    }
}

// See Worker.h
void Worker::Stop() {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;
    running.store(false);
    shutdown(server_fd.load(), SHUT_RDWR);
}

// See Worker.h
void Worker::Join() {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;
    pthread_join(thread, nullptr);
}

inline void modifyEpollContext(int epollfd, int operation, int fd, uint32_t events) {
    struct epoll_event event;
    event.events = events;
    event.data.fd = fd;

    if (epoll_ctl(epollfd, operation, fd, &event) == -1) {
        throw std::runtime_error("Failed to add an event for socket");
    }
}


// See Worker.h
void Worker::OnRun(int server_socket) {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;

    int epollfd;
    struct epoll_event events[MAXEVENTS];

    epollfd = epoll_create(MAXCONN);
    if (epollfd < 0) {
        throw std::runtime_error("Failed to create epoll context");
    }

    modifyEpollContext(epollfd, EPOLL_CTL_ADD, server_socket, EPOLLEXCLUSIVE | EPOLLERR | EPOLLIN | EPOLLHUP);

    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t sinSize = sizeof(struct sockaddr_in);

    while(running.load()) {

        int n = epoll_wait(epollfd, events, MAXEVENTS, -1);
        if (n == -1) {
            throw std::runtime_error("Failed to wait");
        }

        for (int i = 0; i < n; i++) {

            if (events[i].data.fd == server_socket) {

                if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR) {
                    close(server_socket);
                    std::cout << "EPOLLHUP || EPOLLERR" << std::endl;
                    return;
                }

                while (1) {

                    client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &sinSize);

                    if (client_socket == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            break;
                        else
                            throw std::runtime_error("Accept failed");
                    } else {
                        make_socket_non_blocking(client_socket);
                        modifyEpollContext(epollfd, EPOLL_CTL_ADD, client_socket, EPOLLERR | EPOLLIN | EPOLLHUP);

                        auto connect = std::unique_ptr<Connection>(new Connection(client_socket, pStorage));
                        connections[client_socket] = std::move(connect);
                    }
                }
            } else {
                if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR) {
                    std::cout << "Client socket error or closed" << std::endl;
                    close(events[i].data.fd);

                } else if (events[i].events & EPOLLIN) {

                    auto item = connections.find(events[i].data.fd);

                    if (item == connections.end()) {
                        throw std::runtime_error("Error with find socket in hash table");
                    } else {
                        item->second->Read();

                        if (item->second->getState() == Connection::State::st_send) {
                            modifyEpollContext(epollfd, EPOLL_CTL_MOD, item->first, EPOLLERR | EPOLLOUT | EPOLLHUP);
                        } else if (item->second->getState() == Connection::State::st_closed) {
                            close(item->first);
                            connections.erase(item);
                        }
                    }
                } else if (events[i].events & EPOLLOUT) {

                    auto item = connections.find(events[i].data.fd);

                    if (item == connections.end()){
                        throw std::runtime_error("Error");
                    } else {
                        item->second->Send();

                        if (item->second->getState() == Connection::State::st_read) {
                            modifyEpollContext(epollfd, EPOLL_CTL_MOD, item->first, EPOLLERR | EPOLLIN | EPOLLHUP);
                        } else if (item->second->getState() == Connection::State::st_closed) {
                            close(item->first);
                            connections.erase(item);
                        }
                    }
                }
            }
        }
    }
    close(epollfd);
}

} // namespace NonBlocking
} // namespace Network
} // namespace Afina
