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

    //usleep(10000000);
    memset(buff, 0, buff_size);

    ssize_t n = recv(connection_fd, buff, buff_size, 0);

    if (n < 0)
        throw std::runtime_error("Error with read from socket");
    else if (n == 0) {
        state = Connection::State::socket_closed;
        return;
    }

    input_data.append(buff, n);

    if (!parser.parseComplete()) {
        try {
            parser.Parse(&input_data[parsed], n, parsed);
        } catch (std::invalid_argument &e) {
            input_data.clear();
            output_data = e.what();
            output_data += ":ERROR";
            state = Connection::State::socket_send;
            return;
        }
    }

    if (parser.parseComplete()) {

        uint32_t args_size, rest;
        auto command = parser.Build(args_size);

        rest = args_size;
        if (args_size != 0)
            rest += msg_end.size();

        if (input_data.size() - parsed - rest < 0){
            return;
        } else {
            command->Execute(*pStorage, input_data.substr(parsed, args_size), output_data);
            output_data += msg_end;

            parser.Reset();
            input_data = input_data.substr(parsed + rest);
            parsed = 0;
            state = Connection::State::socket_send;
            return;
        }
    }
}

void Connection::Send() {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;

    ssize_t n = send(connection_fd, output_data.data(), output_data.size(), 0);

    if (n < 0) {
        throw std::runtime_error("Error with send in socket");
    } else if (n == 0) {
        state = Connection::State::socket_closed;
        return;
    } else if (n < output_data.size()) {
        output_data = output_data.substr(n);
        return;
    } else {
        output_data.clear();
        if (!input_data.empty()) {

            try {
                parser.Parse(input_data, parsed);
            } catch (std::invalid_argument &e) {
                input_data.clear();
                output_data = e.what();
                output_data += ":ERROR\r\n";
                return;
            }

            if (parser.parseComplete()) {
                uint32_t args_size, rest;
                auto command = parser.Build(args_size);

                rest = args_size;
                if (args_size != 0)
                    rest += msg_end.size();

                if (input_data.size() - parsed - rest < 0) {
                    state = Connection::socket_read;
                    return;
                } else {
                    command->Execute(*pStorage, input_data.substr(parsed, args_size), output_data);
                    output_data += msg_end;

                    parser.Reset();
                    input_data = input_data.substr(parsed + rest);
                    parsed = 0;
                    return;
                }
            } else {
                state = Connection::socket_read;
                return;
            }
        } else {
            state = Connection::socket_read;
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

