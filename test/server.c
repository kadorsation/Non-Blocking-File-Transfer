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
#define min(x,y) (x < y?x:y)

struct user
{
    char user_name[1025];
    struct sockaddr_in clientInfo;
    int flag;
    int file_send;
    char filename[1025];
    int fileptr;
    int s_r;
    int pkts;
    int recpkt;
    int lastpkt;
    struct user_data * data_ptr;
};

struct user_data{
    char user_name[1025];
    char file[20][1025];
    int file_num;
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
        memset(client[i].user_name, '\0', sizeof(client[i].user_name));
        client[i].file_send = 0;
        client[i].fileptr = 0;
        client[i].s_r = 0;
    }

    struct user_data data[10];
    for(i = 0; i < 10; i++){
        memset(data[i].user_name, '\0', sizeof(data[i].user_name));
        for (j = 0; j < 20; j++)
        {
            memset(data[i].file[j], '\0', sizeof(data[i].file[j]));
        }
        data[i].file_num = 0;
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


                                            //reset datas
                    memset(client[i].user_name, '\0', sizeof(client[i].user_name));
                    if(!read(newfd, client[i].user_name, sizeof(client[i].user_name))){
                        printf("read success\n");
                    }
                    else{
                        printf("read fail\n");
                    }
                    client[i].file_send = 0;
                    client[i].fileptr = 0;


                    printf("dir: %s\n", client[i].user_name);
                    check = mkdir(client[i].user_name, MODE);
                    if(check == 0) {        //new user name
                        printf("dir OK\n");
                        for(j = 0; j < 10; j++){
                            if (data[j].user_name[0] == '\0'){
                                sprintf(data[j].user_name, "%s", client[i].user_name);
                                client[i].data_ptr = &data[j];
                                break;
                            }
                        }
                    }
                    else{                   //old user name 
                        printf("dir ERR\n");
                        for(j = 0; j < 10; j++){
                            printf("j: %d\n", j);
                            if(strcmp(data[j].user_name, client[i].user_name) == 0){
                                client[i].data_ptr = &data[j];
                                break;
                            }
                        }
                    }
                    
                    memset(buffer, '\0', sizeof(buffer));
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
                max = newfd;                // for select
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
                if(n == 0 || buffer == "exit\n"){
                    close(sockfd); 
                    FD_CLR(sockfd, &all); 
                    client[i].flag = -1; 
                    client[i].file_send = 0;
                    client[i].fileptr = 0;
                    printf("%s leave.\n", client[i].user_name);
                    memset(client[i].user_name, '\0', sizeof(client[i].user_name));
                }
                else {
                    /*printf("buffer: %s\n", buffer);
                    char putcheck[3];
                    char putcheck1[3];
                    strncpy(putcheck, buffer, 3);
                    strncpy(putcheck1, buffer, 3);
                    printf("putcheck: %s\n", putcheck);
                    printf("putcheck1: %s\n", putcheck1);
                    printf("CMD: %s\n", putcheck);*/


                    //put request receive
                    if(buffer[0] == 'p' && buffer[1] == 'u' && buffer[2] == 't'){

                        char* pch = strtok(buffer, " ");
                        pch = strtok(NULL, " ");
                        strcpy(client[i].filename, pch);
                        printf("fliename: %s\n", client[i].filename);

                        pch = strtok(NULL, " ");
                        printf("size: %s\n", pch);
                        int size = atoi(pch);
                        printf("size: %d\n", size);

                        client[i].pkts = (size / 1025);
                        client[i].lastpkt = 1025;
                        if(size % 1025 != 0){ 
                            client[i].pkts++;
                            client[i].lastpkt = size % 1025;
                            printf("lastpktsize: %d\n", client[i].lastpkt);
                        }
                        client[i].recpkt = 0;


                        client[i].file_send = client[i].data_ptr->file_num;     //from 0
                        strcpy(client[i].data_ptr->file[client[i].data_ptr->file_num], client[i].filename);
                        client[i].data_ptr->file_num++;   
                        continue; 
                    }
                    else{
                        //DIR *dir = opendir(client[i].user_name);
                        //int pktsize = min(strlen(buffer), 1025);
                        client[i].recpkt++;

                        char usedir[1025];
                        memset(usedir, '\0', sizeof(usedir));
                        sprintf(usedir, "%s/%s", client[i].user_name, client[i].filename);     
                        printf("Use dir:%s\n", usedir);


                        FILE *fp = fopen(usedir, "ab+");
                        if(client[i].recpkt != client[i].pkts){
                            fwrite(buffer, 1, sizeof(buffer), fp);
                        }
                        else{
                            fwrite(buffer, 1, client[i].lastpkt, fp);
                        }
                        fclose(fp);
                        printf("input: %s\n\n", buffer);
                        //closedir(dir);

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
