#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
 
int main()
{
 
    char str[100];
    int MasterSock, SlaveSock;
    socklen_t len;

    struct sockaddr_in servaddr, cliaddr;
 
    MasterSock = socket(AF_INET, SOCK_STREAM, 0);

    setsockopt(MasterSock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
 
    bzero( &servaddr, sizeof(servaddr));
 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(1234);
 
    bind(MasterSock, (struct sockaddr *) &servaddr, sizeof(servaddr));
 
    listen(MasterSock, 10);


    char buff[20];
    len = sizeof(cliaddr);

    char strr[] = "HELLdgdfgsdgsrgseO!\r\n";

    SlaveSock = accept(MasterSock, (struct sockaddr*) &cliaddr, &len);

    printf("connection from %s, port %d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)),
           ntohs(cliaddr.sin_port));
    int i = 0;
    while(i < 10)
    {

        if (write(SlaveSock, strr, strlen(strr)+1) <= 0)
            break;
        printf("%s", strr);
        i += 1;
    }
    for (i = 0; i < 10000000; i++){
        len += 1;
    }
    shutdown(SlaveSock, 2);
    close(SlaveSock);
}
