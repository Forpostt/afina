#include "Connection.h"

#include <iostream>
#include <memory>
#include <cstring>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <afina/execute/Command.h>
#include <afina/Storage.h>
#include <protocol/Parser.h>

std::string msg_end = "\r\n";

namespace Afina{
namespace Network{
namespace NonBlocking{

Connection::Connection(int fd, std::shared_ptr <Afina::Storage> ps): connection_fd(fd), pStorage(ps) {};

Connection::~Connection() {
    input_data.clear();
    output_data.clear();
    parser.Reset();
};

void Connection::Read() {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;

    const size_t buff_size = 1024;
    char buff[buff_size];
    ssize_t n;

    memset(buff, 0, buff_size);

    n = recv(connection_fd, buff, buff_size, 0);
    if (n < 0)
        throw std::runtime_error("Error with read from socket");
    else if (n == 0) {
        state = Connection::State::st_closed;
        return;
    }

    input_data.append(buff, n);

    if (!parser.parseComplete()) {
        parser.Parse(&input_data[parsed], n, parsed);
    }

    if (parser.parseComplete()) {
        uint32_t args_size, rest;
        auto command = parser.Build(args_size);

        rest = args_size;
        if (args_size != 0)
            rest += msg_end.size();

        if (input_data.size() - parsed - rest < 0)
            return;
        else {
            command->Execute(*pStorage, input_data.substr(parsed, args_size), output_data);
            output_data += msg_end;

            parser.Reset();
            input_data = input_data.substr(parsed + rest);
            parsed = 0;
            state = Connection::State::st_send;
            return;
        }
    }
}

void Connection::Send() {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;

    ssize_t n = 0;
    n = send(connection_fd, output_data.data(), output_data.size(), 0);

    if (n < 0) {
        throw std::runtime_error("Error with send in socket");
    } else if (n == 0) {
        state = Connection::State::st_closed;
        return;
    } else if (n < output_data.size()) {
        return;
    } else {
        output_data.clear();
        if (!input_data.empty()) {

            parser.Parse(&input_data[0], input_data.size(), parsed);

            if (parser.parseComplete()) {
                uint32_t args_size, rest;
                auto command = parser.Build(args_size);

                rest = args_size;
                if (args_size != 0)
                    rest += msg_end.size();

                if (input_data.size() - parsed - rest < 0) {
                    state = Connection::st_read;
                    return;
                } else {
                    command->Execute(*pStorage, input_data.substr(parsed, args_size), output_data);
                    parser.Reset();
                    input_data = input_data.substr(parsed + rest);
                    parsed = 0;
                    return;
                }
            } else {
                state = Connection::st_read;
                return;
            }
        } else {
            state = Connection::st_read;
            return;
        }
    }
}

Connection::State Connection::getState() const {
    return state;
}


} // namespace NonBlocking
} // namespace Network
} // namespace Afina

