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
    listener = socket(AF_INET , SOCK_STREAM , 0);
    int newfd;
    socklen_t len;
    char buffer[1025], hello[1025], check[1025], input[1025], old_name[1025], tell[1025], tell_item[1025];
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
        strcpy(client[i].user_name, "anonymous");
    }


    while(1){
        select_fd = all;
        nready = select(max+1,&select_fd,NULL,NULL,NULL);
        if (FD_ISSET(listener, &select_fd)) {    //new connection
            printf("New client!\n");
            
            for (i = 0; i < 10; i++){ 
                if (client[i].flag < 0) { 
                    len = sizeof(client[i].clientInfo);
                    newfd = accept(listener, (struct sockaddr *) &client[i].clientInfo, &len);
                    client[i].flag = newfd;
                    /*Hello Message*/
                    memset(buffer, '\0', sizeof(buffer));
                    sprintf(buffer, "[Server] Hello, anonymous! From: %s:%d\n", inet_ntop(AF_INET, &client[i].clientInfo.sin_addr, hello, INET_ADDRSTRLEN), ntohs(client[i].clientInfo.sin_port));
                    write(newfd, buffer, sizeof(buffer));
                    for(j = 0; j < 10 ; j++){
                        if(client[j].flag != newfd){
                            write(client[j].flag, "[Server] Someone is coming!\n", 28);
                        }
                    }
                    /*Hello Message-end*/
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
            if (FD_ISSET(sockfd, &select_fd)){  //因為第81行所以sockfd現在是此cliend的socket值
                memset(buffer, '\0', sizeof(buffer));
                n = read(sockfd, buffer, 1025);
                printf("%s", buffer);
                if(n == 0 || buffer == "exit\n"){
                    close(sockfd); 
                    FD_CLR(sockfd, &all); 
                    client[i].flag = -1; 
                    /*Offline Message*/
                    for(j = 0; j < 10; j++){
                        if(client[j].flag >0){
                            memset(buffer, '\0', sizeof(buffer));
                            sprintf(buffer, "[Server] %s is offline.\n", client[i].user_name);
                            write(client[j].flag, buffer, sizeof(buffer));
                        }
                    }
                    /*Offline Message-end*/
                }
                else {
                    /*Who Message*/
                    if(strcmp(buffer, "who\n") == 0){
                        for(j = 0; j < 10; j++){
                            if(client[j].flag > 0){
                                memset(buffer, '\0', sizeof(buffer));
                                if(i == j){
                                    sprintf(buffer, "[Server] %s %s:%d ->me\n", client[j].user_name, inet_ntop(AF_INET, &client[j].clientInfo.sin_addr, hello, INET_ADDRSTRLEN), ntohs(client[j].clientInfo.sin_port));
                                }
                                else{
                                    sprintf(buffer, "[Server] %s %s:%d\n", client[j].user_name, inet_ntop(AF_INET, &client[j].clientInfo.sin_addr, hello, INET_ADDRSTRLEN), ntohs(client[j].clientInfo.sin_port));
                                }
                                write(client[i].flag, buffer, sizeof(buffer));
                            }
                        }
                        continue;
                    }
                    /*Who Message-end*/


                    memset(check, '\0', sizeof(check));
                    memset(input, '\0', sizeof(input));
                    memcpy(check, buffer, 5);
                    strcpy(input, buffer + 5);
                    memset(buffer, '\0', sizeof(buffer));
                    if(input[strlen(input) - 1] == '\n'){
                        input[strlen(input) - 1] = '\0';
                    }


                    /*Change Username Message*/
                    if(strcmp(check, "name ") == 0){
                        if(strlen(input) > 12 || strlen(input) < 2 ){
                            sprintf(buffer, "[Server] ERROR: Username can only consists of 2~12 English letters.\n");
                            write(client[i].flag, buffer, sizeof(buffer));
                            continue;
                        }
                        if(strcmp(input, "anonymous") == 0){
                            sprintf(buffer, "[Server] ERROR: Username cannot be anonymous.\n");
                            write(client[i].flag, buffer, sizeof(buffer));
                            continue;
                        }
                        int err = 0;
                        for(j = 0; j < strlen(input); j++){
                            if( input[j] < 65 || input[j] > 122 || (input[j] > 90 && input[j] < 97) ){
                                sprintf(buffer, "[Server] ERROR: Username can only consists of 2~12 English letters.\n");
                                write(client[i].flag, buffer, sizeof(buffer));
                                err = 1;
                                break;
                            }
                        }
                        for(j = 0; j < 10; j++){
                            if(strcmp(input, client[j].user_name) == 0 && client[j].flag > 0){
                                sprintf(buffer, "[Server] ERROR: <NEW USERNAME> has been used by others.\n");
                                write(client[i].flag, buffer, sizeof(buffer));
                                err = 1;
                                break;
                            }
                        }
                        if(err == 0){
                            memset(old_name, '\0', sizeof(old_name));
                            strcpy(old_name ,client[i].user_name);
                            memset(client[i].user_name, '\0', sizeof(client[i].user_name));
                            strcpy(client[i].user_name, input);
                            sprintf(buffer, "[Server] You're now known as %s.\n", client[i].user_name);
                            write(client[i].flag, buffer, sizeof(buffer));
                            for(j = 0; j < 10; j++){
                                if(client[j].flag > 0 && j != i){
                                    sprintf(buffer, "[Server] %s is now known as %s.\n", old_name, client[i].user_name);
                                    write(client[j].flag, buffer, sizeof(buffer));
                                }
                            }
                        }
                        continue;
                    }
                    /*Change Username Message-end*/

                    /*Private Message*/
                    if(strcmp(check, "tell ") == 0){
                        if(strcmp(client[i].user_name, "anonymous") == 0){
                            sprintf(buffer, "[Server] ERROR: You are anonymous.\n");
                            write(client[i].flag, buffer, sizeof(buffer));
                            continue;
                        }
                        strcpy(tell, strtok(input, " "));
                        if(tell[strlen(tell) - 1] == '\n'){
                            tell[strlen(tell) - 1] = '\0';
                        }
                        if(strcmp(tell, "anonymous") == 0){
                            sprintf(buffer, "[Server] ERROR: The client to which you sent is anonymous.\n");
                            write(client[i].flag, buffer, sizeof(buffer));
                            continue;
                        }
                        strcpy(tell_item, input + strlen(tell) + 1);
                        if(tell_item[strlen(tell_item) - 1] == '\n'){
                            tell_item[strlen(tell_item) - 1] = '\0';
                        }
                        int err = 0;
                        for(j = 0; j < 10 ; j++){
                            if(strcmp(tell, client[j].user_name) == 0 && client[j].flag > 0){
                                err = 1;
                                sprintf(buffer, "[Server] SUCCESS: Your message has been sent.\n");
                                write(client[i].flag, buffer, sizeof(buffer));
                                memset(buffer, '\0', sizeof(buffer));
                                sprintf(buffer, "[Server] %s tell you %s\n", client[i].user_name, tell_item);
                                write(client[j].flag, buffer, sizeof(buffer));
                                break;
                            }
                        }
                        if(err == 0){
                            sprintf(buffer, "[Server] ERROR: The receiver doesn't exist.\n");
                            write(client[i].flag, buffer, sizeof(buffer));
                        }
                        continue;
                    }
                    /*Private Message-end*/

                    /*Broadcast Message*/
                    if(strcmp(check, "yell ") == 0){
                        for(j = 0; j < 10; j++){
                            if(client[j].flag > 0){
                                sprintf(buffer, "[Server] %s yell %s\n", client[i].user_name, input);
                                write(client[j].flag, buffer, sizeof(buffer));
                            }
                        }
                        continue;
                    }
                    /*Broadcast Message-end*/

                    /*Error Command*/
                    sprintf(buffer, "[Server] ERROR: Error command.\n");
                    printf("%s\n", buffer);
                    write(client[i].flag, buffer, sizeof(buffer));
                    /*Error Command-end*/
                }
                if (--nready <= 0) {
                    break; 
                }
            }
        }
    }
    return 0;
}