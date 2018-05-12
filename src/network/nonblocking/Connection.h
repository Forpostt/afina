#ifndef AFINA_CONNECTION_H
#define AFINA_CONNECTION_H

#include "afina/Storage.h"
#include "protocol/Parser.h"

namespace Afina {
namespace Network {
namespace NonBlocking {

class Connection {
public:

    /**
     * All possible states of the connection
     */
    enum State{
        socket_read,
        socket_send,
        socket_closed
    };

    Connection(int fd, std::shared_ptr<Afina::Storage> ps);
    ~Connection();

    /**
     * The method is used when the corresponding socket is ready for reading.
     * In case of epoll use when event is EPOLLIN.
     */
    void Read();

    /**
     * The method is used when the corresponding socket is ready for writing.
     * In case of epoll use when event is EPOLLOUT.
     */
    void Send();

    /**
     * Returns the current state of the connection
     */
    Connection::State getState() const;

private:
    int connection_fd;

    Afina::Protocol::Parser parser;
    size_t parsed = 0;

    std::string input_data;
    std::string output_data;

    static const size_t buff_size = 1024;
    char buff[buff_size];

    std::shared_ptr<Afina::Storage> pStorage;

    Connection::State state = State::socket_read;
};

} // namespace NonBlocking
} // namespace Network
} // namespace Afina
#endif //AFINA_CLIENT_H
