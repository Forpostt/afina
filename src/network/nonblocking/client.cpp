#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>

#include <string>
#include <strings.h>

int main(int argc,char **argv) {
    int Socket;
    int n;
    char recvline[100];
    struct sockaddr_in servaddr;

    Socket = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof servaddr);

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080);

    inet_pton(AF_INET, "127.0.0.1", &(servaddr.sin_addr));

    connect(Socket, (struct sockaddr *)&servaddr, sizeof(servaddr));

    //std::string out = "set foo 0 0 6\r\nfooval\r\n";
    std::string out = "asd";
    send(Socket, out.data(), out.size(), 0);

    usleep(10000000);

    shutdown(Socket, 2);
    close(Socket);
}
