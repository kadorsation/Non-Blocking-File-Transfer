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
#include <fcntl.h> 
#define min(x,y) (x < y?x:y)

int filesize(FILE* fp){
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    rewind(fp); 
    return size;
}

int main(int argc, char *argv[])
{
    if(argc != 4){
        printf("argv error\n");
        return 0;
    }
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

    int flags;
    char sendline[1025], recvline[1025], inputline[1025];
    memset(sendline, '\0', sizeof(sendline));
    sprintf(sendline, "%s", argv[3]);
    write(sockfd, sendline, sizeof(sendline));

    while(1){
        FD_SET(fileno(stdin), &all);
        FD_SET(sockfd, &all);
        
        flags = fcntl(fileno(stdin),F_GETFL);
        fcntl(fileno(stdin), F_SETFL, flags|O_NONBLOCK);
        flags = fcntl(sockfd,F_GETFL);
        fcntl(sockfd, F_SETFL, flags|O_NONBLOCK);

        select((sockfd+1), &all, NULL, NULL, NULL);
        if(FD_ISSET(sockfd, &all)){
            memset(recvline,'\0',1025);
            read(sockfd, recvline, 1025);
            fputs(recvline, stdout);
        }


        memset(inputline,'\0',1025);
        fgets(inputline, 1025, stdin);
        if(strcmp(inputline, "exit\n") == 0){
            close(sockfd);
            FD_ZERO(&all);
            return 0;
        }

        //sleep
        char pch[5];
        strncpy(pch, inputline, 5);
        if(strcmp(pch, "sleep") == 0){
            char* pch2 = strtok(inputline, " ");
            pch2 = strtok(NULL, " ");
            printf("The client starts to sleep.\n");
            int slt = atoi(pch2);
            for(int i = 1; i <= slt; i++){
                printf("Sleep %d\n", i);
                sleep(1);
            }
            printf("Client wakes up.\n");
            continue;
        }

        //put
        char pch1[3];
        strncpy(pch1, inputline, 3);
        printf("input: %s\n", inputline);
        printf("pch1: %s\n", pch1);
        if(strcmp(pch1, "put") == 0){
            char* pch2 = strtok(inputline, " ");
            pch2 = strtok(NULL, " ");
            pch2[strlen(pch2)-1] = '\0';
            printf("[Upload] %s Start!\n", pch2);
            FILE *fp = fopen(pch2, "rb");

            //send file info
            int size = filesize(fp);
            memset(sendline, '\0', sizeof(sendline));
            sprintf(sendline, "put %s %d", pch2, size);
            write(sockfd, sendline, sizeof(sendline));
            //printf("sendline: %s\n", sendline);


            int pkts = (size / 1025);
            int lastpktsize = 1025;
            if(size % 1025 != 0){ 
                pkts++;
                lastpktsize = size % 1025;
                //printf("lastpktsize: %d\n", lastpktsize);
            }

            printf("num of pkts: %d\n", pkts);
            int i = 0;
            while(i < pkts){
                memset(sendline, '\0', sizeof(sendline));
                fread(sendline, 1, 1025, fp);
                //int pktsize = min(strlen(sendline), 1025);
                if(write(sockfd, sendline, sizeof(sendline))){
                    i++;
                    FILE *fp2 = fopen("wr", "a+");
                    if(i == pkts){
                        fwrite(sendline, 1, lastpktsize, fp2);
                        fclose(fp2);
                        fprintf(stdout, "ACK %d Size: %d", i, lastpktsize);
                        fflush(stdout);
                        printf("\n");
                        //printf("%s\n", sendline);
                    }
                    else{
                        fwrite(sendline, 1, sizeof(sendline), fp2);
                        fclose(fp2);
                        fprintf(stdout, "ACK %d Size: %lu", i, sizeof(sendline));
                        fflush(stdout);
                        printf("\r");
                        //printf("%s\n", sendline);
                    }
                }
                sleep(1);
            }


            fclose(fp);
            printf("[Upload] %s Finish!\n", pch2);
        }


    }
    return 0;
}
