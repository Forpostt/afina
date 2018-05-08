#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
 
int main(int argc,char **argv)
{
    int Socket;
    int n;
    char recvline[100];
    struct sockaddr_in servaddr;
 
    Socket = socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof servaddr);
 
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(1234);
 
    inet_pton(AF_INET,"127.0.0.1",&(servaddr.sin_addr));
 
    connect( Socket, (struct sockaddr *)&servaddr,sizeof(servaddr) );
    bzero( recvline, 100);
    int i = 0;
    while(1) {

        if (recv( Socket, &recvline[0], 100, 0) <= 0)
            break;

        printf("%s",recvline);


    }
    shutdown(Socket, 2);
    close(Socket);
}
