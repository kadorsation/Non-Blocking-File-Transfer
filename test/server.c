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
#include <errno.h>
#include <netdb.h>

#define MODE (S_IRWXU | S_IRWXG | S_IRWXO)
//#define min(x,y) (x < y?x:y)

int filesize(FILE* fp){
    printf("s0\n");
    fseek(fp, 0, SEEK_END);
    printf("s1\n");
    int size = ftell(fp);
    printf("s2\n");
    rewind(fp); 
    printf("s3\n");
    return size;
}

struct user
{
    char user_name[1025];
    struct sockaddr_in clientInfo;
    int flag;
    int file_send;      //the file is using now
    char filename[1025];
    int fileptr;        //the file has been send
    int s_r;            //1 is s 2 is r
    int pkts;
    int recpkt;
    int sendpkt;
    int lastpkt;
    struct user_data * data_ptr;
    FILE *fp;
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
    //FD_SET(listener, &all);
    int flags = fcntl(listener,F_GETFL);
    fcntl(listener, F_SETFL, flags|O_NONBLOCK);

    max = listener;

    struct user client[10];
    for(i = 0; i < 10; i++){
        client[i].flag = -1;
        memset(client[i].user_name, '\0', sizeof(client[i].user_name));
        client[i].file_send = 0;
        client[i].fileptr = 0;
        client[i].s_r = 0;
        client[i].data_ptr = NULL;
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
        /*select_fd = all;
        nready = select(max+1,&select_fd,NULL,NULL,NULL);*/
        //if (FD_ISSET(listener, &select_fd)) {    //new connection
            //printf("into listener\n");
        newfd = accept(listener, (struct sockaddr *) &client[i].clientInfo, &len);
        if(newfd > 0){
            //printf("newfd: %d\n", newfd); 
            for (i = 0; i < 10; i++){ 
                if (client[i].flag < 0) { 
                    //printf("Someone comming\n");
                    len = sizeof(client[i].clientInfo);
                    //newfd = accept(listener, (struct sockaddr *) &client[i].clientInfo, &len);
                    client[i].flag = newfd;

                    int flags = fcntl(newfd,F_GETFL);
                    fcntl(newfd, F_SETFL, flags|O_NONBLOCK);


                                            //reset datas
                    memset(client[i].user_name, '\0', sizeof(client[i].user_name));
                    while(!(read(newfd, client[i].user_name, sizeof(client[i].user_name)) > 0));
                    client[i].file_send = 0;
                    client[i].fileptr = 0;


                    //printf("dir: %s\n", client[i].user_name);
                    check = mkdir(client[i].user_name, MODE);
                    if(check == 0) {        //new user name
                        //printf("dir OK\n");
                        for(j = 0; j < 10; j++){
                            if (data[j].user_name[0] == '\0'){
                                sprintf(data[j].user_name, "%s", client[i].user_name);
                                client[i].data_ptr = &data[j];
                                break;
                            }
                        }
                    }
                    else{                   //old user name 
                        //printf("dir ERR\n");
                        for(j = 0; j < 10; j++){
                            //printf("j: %d\n", j);
                            if(strcmp(data[j].user_name, client[i].user_name) == 0){
                                client[i].data_ptr = &data[j];
                                break;
                            }
                        }
                    }
                    
                    memset(buffer, '\0', sizeof(buffer));
                    sprintf(buffer, "Welcome to the dropbox-like server: %s\n", client[i].user_name);
                    write(client[i].flag, buffer, sizeof(buffer));
                    //FD_SET(client[i].flag, &all);
                    break; 
                } 
            }
            newfd = 0;
            /*if (i >= 10){ 
                continue;
            }*/
            /*if (newfd > max){ 
                max = newfd;                // for select
            }
            if (--nready <= 0){ 
                continue;
            }*/

            /*for(j = 0; j < 10; j++){
                printf("%d : %s\n", j, data[j].user_name);
            }*/
        }
        //}
        //printf("loop!\n");

        for( i = 0; i < 10; i++){
            sockfd = client[i].flag;
            //printf("%d\n", client[i].flag);
            if(sockfd < 0){
                continue;
            }



            memset(buffer, '\0', sizeof(buffer));
            n = read(sockfd, buffer, 1025);
            if(n == 0 || buffer == "exit\n"){
                close(sockfd); 
                client[i].flag = -1; 
                client[i].file_send = 0;
                client[i].fileptr = 0;
                //printf("%s leave.\n", client[i].user_name);
                memset(client[i].user_name, '\0', sizeof(client[i].user_name));
            }
            else if(n < 0){
                if(errno == EAGAIN && client[i].s_r == 2){
                    continue;
                }
            }
            else{
                /*printf("read something, N : %ld\n", n);
                if(errno == EAGAIN){
                    continue;
                }*/
                //printf("receive something\n");
                //printf("buffer: %s\n", buffer);
                /*char putcheck[3];
                char putcheck1[3];
                strncpy(putcheck, buffer, 3);
                strncpy(putcheck1, buffer, 3);
                printf("putcheck: %s\n", putcheck);
                printf("putcheck1: %s\n", putcheck1);
                printf("CMD: %s\n", putcheck);*/

                //printf("buffer: %s\n", buffer);
                //put request receive
                if(buffer[0] == 'p' && buffer[1] == 'u' && buffer[2] == 't'){

                    char* pch = strtok(buffer, " ");
                    pch = strtok(NULL, " ");
                    strcpy(client[i].filename, pch);
                    //printf("fliename: %s\n", client[i].filename);
                    pch = strtok(NULL, " ");
                    //printf("size: %s\n", pch);
                    int size = atoi(pch);
                    //printf("size: %d\n", size);
                    client[i].pkts = (size / 1025);
                    client[i].lastpkt = 1025;
                    if(size % 1025 != 0){ 
                        client[i].pkts++;
                        client[i].lastpkt = size % 1025;
                        //printf("lastpktsize: %d\n", client[i].lastpkt);
                    }
                    client[i].recpkt = 0;
                    client[i].s_r = 2;
                    //set file data
                    client[i].file_send = client[i].data_ptr->file_num;     //from 0
                    strcpy(client[i].data_ptr->file[client[i].data_ptr->file_num], client[i].filename);
                    //printf("put file name: %s\n", client[i].data_ptr->file[client[i].data_ptr->file_num]);

                }
                else{
                    //DIR *dir = opendir(client[i].user_name);
                    //int pktsize = min(strlen(buffer), 1025);
                    client[i].recpkt++;
                    //printf("recpkt: %d\n", client[i].recpkt);
                    char usedir[1025];
                    memset(usedir, '\0', sizeof(usedir));
                    sprintf(usedir, "%s/%s", client[i].user_name, client[i].filename);     
                    //printf("Use dir:%s\n", usedir);


                    FILE *fp = fopen(usedir, "ab+");
                    if(client[i].recpkt != client[i].pkts){
                        fwrite(buffer, 1, sizeof(buffer), fp);
                    }
                    else{
                        fwrite(buffer, 1, client[i].lastpkt, fp);
                        client[i].data_ptr->file_num++; 
                        client[i].s_r = 0;
                        client[i].fileptr++;
                    }
                    fclose(fp);
                    //printf("input: %s\n\n", buffer);
                    //closedir(dir);
                    
                }
                continue;
                    
            }
            //put end

            

            if((client[i].data_ptr->file_num > client[i].fileptr) && (client[i].s_r == 0) && (client[i].flag > 0)){
                client[i].s_r = 1;
                //printf("%s is no file %d\n", client[i].user_name, i);
                char usedir[1025];
                memset(usedir, '\0', sizeof(usedir));
                //printf("client[i].data_ptr->file_num %d,client[i].fileptr %d\n", client[i].data_ptr->file_num, client[i].fileptr);
                sprintf(usedir, "%s/%s", client[i].user_name, client[i].data_ptr->file[client[i].fileptr]);
                //printf("user_name: %s; file: %s; usedir: %s\n", client[i].user_name, client[i].data_ptr->file[client[i].fileptr], usedir);
                client[i].fp = fopen(usedir, "rb");
                int size = filesize(client[i].fp);
                client[i].sendpkt = 0;
                client[i].pkts = (size / 1025);
                client[i].lastpkt = 1025;
                if(size % 1025 != 0){ 
                    client[i].pkts++;
                    client[i].lastpkt = size % 1025;
                    //printf("lastpktsize: %d\n", client[i].lastpkt);
                }
                memset(buffer, '\0', sizeof(buffer));
                sprintf(buffer, "put %s %d", client[i].data_ptr->file[client[i].file_send], size);     //file_name file_size
                n = write(sockfd, buffer, sizeof(buffer));
            }

            if(client[i].s_r == 1 && client[i].flag > 0){
                printf("send pkt %d to %d\n", client[i].sendpkt, i);
                memset(buffer, '\0', sizeof(buffer));
                fread(buffer, 1, 1025, client[i].fp);

                //FILE *fp2 = fopen("wr", "a+");
                //fwrite(buffer, 1, sizeof(buffer), fp2);
                //fclose(fp2);

                n = write(sockfd, buffer, sizeof(buffer));
                if(n > 0){
                    client[i].sendpkt++;
                    if(client[i].sendpkt == client[i].pkts){
                        printf("The file close\n");
                        fclose(client[i].fp);
                        client[i].s_r = 0;
                        client[i].sendpkt = 0;
                        client[i].pkts = 0;
                        client[i].lastpkt = 0;
                        client[i].fileptr++;
                        printf("client[i].data_ptr->file_num %d,client[i].fileptr %d, i = %d\n", client[i].data_ptr->file_num, client[i].fileptr, i);
                    }
                }
                //sleep(1);
            } 

        }//for end

    }
    return 0;
}
