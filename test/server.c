#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>

struct user
{
    char user_name[1025];
    struct sockaddr_in clientInfo;
    int flag;
};

int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        return 0;
    }
    fd_set all;
    fd_set select_fd;
    FD_ZERO(&all);
    FD_ZERO(&select_fd);
    int max, maxi = -1;
    int i, j;
    ssize_t n; 
    int sockfd;
    int listener;
    int nready;
    int check;

    listener = socket(AF_INET , SOCK_STREAM , 0);

    int newfd;
    socklen_t len;

    char buffer[1025], hello[1025], input[1025];
    int port = atoi (argv[1]);
    struct sockaddr_in serverInfo,clientInfo;

    bzero(&serverInfo,sizeof(serverInfo));

    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(port);

    bind(listener,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
    listen(listener,10);
    FD_SET(listener, &all);

    max = listener;

    struct user client[10];

    for(i = 0; i < 10; i++){
        client[i].flag = -1;
        memset(client[i].user_name, '\0', sizeof(client[i].user_name));
    }


    while(1){
        select_fd = all;
        nready = select(max+1,&select_fd,NULL,NULL,NULL);
        if (FD_ISSET(listener, &select_fd)) {    //new connection
            
            for (i = 0; i < 10; i++){ 
                if (client[i].flag < 0) { 
                    printf("Someone comming\n");
                    len = sizeof(client[i].clientInfo);
                    newfd = accept(listener, (struct sockaddr *) &client[i].clientInfo, &len);
                    client[i].flag = newfd;

                    memset(client[i].user_name, '\0', sizeof(client[i].user_name));
                    memset(buffer, '\0', sizeof(buffer));
                    read(newfd, client[i].user_name, sizeof(client[i].user_name));
                    printf("Welcome to the dropbox-like server: %s\n", client[i].user_name);
                    char dir[1026];
                    sprintf(dir, "/%s", client[i].user_name);
                    printf("dir: %s\n", dir);
                    check = mkdir(dir, S_IRWXU);
                    printf("dirOK\n");
                    sprintf(buffer, "Welcome to the dropbox-like server: %s\n", client[i].user_name);
                    write(client[i].flag, buffer, sizeof(buffer));
                    FD_SET(client[i].flag, &all);
                    break; 
                } 
            }
            if (i >= 10){ 
                continue;
            }
            if (newfd > max){ 
                max = newfd;        /* for select */ 
            }
            if (--nready <= 0){ 
                continue;
            }
        }
        for( i = 0; i < 10; i++){
            sockfd = client[i].flag;
            if(sockfd < 0){
                continue;
            }
            if (FD_ISSET(sockfd, &select_fd)){ 
                memset(buffer, '\0', sizeof(buffer));
                n = read(sockfd, buffer, 1025);
                printf("%s", buffer);
                if(n == 0 || buffer == "exit\n"){
                    close(sockfd); 
                    FD_CLR(sockfd, &all); 
                    client[i].flag = -1; 
                    printf("%s leave.\n", client[i].user_name);
                }
                else {
                    char* pch = strtok(buffer, " ");
                    printf("check: %s\n", pch);
                    pch = strtok(NULL, " ");
                    pch[strlen(pch) - 1] = '\0';
                    printf("input: %s\n", pch);
                    sprintf(buffer, "%s\n", pch);
                    write(client[i].flag, buffer, sizeof(buffer));
                }
                if (--nready <= 0) {
                    break; 
                }
            }
        }
    }
    return 0;
}