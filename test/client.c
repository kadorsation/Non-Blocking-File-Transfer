#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char *argv[])
{
    fd_set all;
    FD_ZERO(&all);
    int sockfd = 0;
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1){
        printf("Fail to create a socket.");
    }
    struct sockaddr_in info;
    bzero(&info,sizeof(info));
    info.sin_family = AF_INET;
    int port = atoi (argv[2]);
    info.sin_addr.s_addr = inet_addr(argv[1]);
    info.sin_port = htons(port);
    int err = connect(sockfd,(struct sockaddr *)&info,sizeof(info));
    if(err==-1){
        printf("Connection error");
    }

    char sendline[1025], recvline[1025];
    while(1){
        FD_SET(fileno(stdin), &all);
        FD_SET(sockfd, &all);
        select((sockfd+1), &all, NULL, NULL, NULL);
        if(FD_ISSET(sockfd, &all)){
            memset(recvline,'\0',1025);
            read(sockfd, recvline, 1025);
            fputs(recvline, stdout);
        }

        if(FD_ISSET(fileno(stdin), &all)){
            fgets(sendline, 1025, stdin);
            if(strcmp(sendline, "exit\n") == 0){
                close(sockfd);
                FD_ZERO(&all);
                return 0;
            }
            write(sockfd, sendline, strlen(sendline));
        }

    }
    return 0;
}