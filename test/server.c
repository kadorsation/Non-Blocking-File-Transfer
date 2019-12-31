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
#include <fcntl.h> 
#include <dirent.h>

#define MODE (S_IRWXU | S_IRWXG | S_IRWXO)

struct user
{
    char user_name[1025];
    struct sockaddr_in clientInfo;
    int flag;
    int sleep;
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

    char buffer[1025], send[1025];
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
        client[i].sleep = 0;
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

                    int flags = fcntl(newfd,F_GETFL);
                    fcntl(newfd, F_SETFL, flags|O_NONBLOCK);

                    memset(client[i].user_name, '\0', sizeof(client[i].user_name));
                    memset(buffer, '\0', sizeof(buffer));
                    read(newfd, client[i].user_name, sizeof(client[i].user_name));
                    printf("dir: %s\n", client[i].user_name);
                    check = mkdir(client[i].user_name, MODE);
                    if(check == 0) {
                        printf("dirOK\n");
                    }
                    else{
                        printf("dir err\n");
                    }
                    
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
                    char* input = strtok(NULL, " ");
                    printf("check: %s\n", pch);
                    printf("input: %s\n", input);
                    printf("buffer: %s\n", buffer);

                    if(strcmp(pch, "put") == 0){
                        memset(send, '\0', sizeof(send));
                        sprintf(send, "[Upload] %s Start!\n", input);
                        write(client[i].flag, send, sizeof(send));
                        printf("%s\n", send);
                        for(int j = 0; j <= 3; j++){
                            memset(send, '\0', sizeof(send));
                            sprintf(send, "[Upload] %s %d\n",input, j);
                            write(client[i].flag, send, sizeof(send));
                        }
                    }
                    
                }
                if (--nready <= 0) {
                    break; 
                }
            }
        }
    }
    return 0;
}
