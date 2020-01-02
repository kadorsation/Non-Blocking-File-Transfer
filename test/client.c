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
#include <errno.h>
//#define min(x,y) (x < y?x:y)

int filesize(FILE* fp){
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    rewind(fp); 
    return size;
}

int bar(int i, int pkts, int *barnum){
    float barcheck = (float)i / (float)pkts;
    //printf("i: %d barcheck: %f\n", i, barcheck);
    if(barcheck *22 > (float)*barnum){
        fprintf(stdout, "Progress : [");
        fflush(stdout);
        *barnum++;
        for(int f = 0; f < 22; f++){
            if((float)f <= barcheck *22){
                fprintf(stdout, "#");
                fflush(stdout);
            }
            else{
                fprintf(stdout, " ");
                fflush(stdout);
            }
        }
        fprintf(stdout, "]");
        fflush(stdout);
        printf("\r");
    }
}

int main(int argc, char *argv[])
{
    if(argc != 4){
        printf("argv error\n");
        return 0;
    }
    //fd_set all;
    //FD_ZERO(&all);
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

    int barnum;
    int lastpkt;

    int flags;
    char sendline[1025], recvline[1025], inputline[1025];
    memset(sendline, '\0', sizeof(sendline));
    sprintf(sendline, "%s", argv[3]);
    write(sockfd, sendline, sizeof(sendline));
    flags = fcntl(fileno(stdin),F_GETFL);
    fcntl(fileno(stdin), F_SETFL, flags|O_NONBLOCK);
    flags = fcntl(sockfd,F_GETFL);
    fcntl(sockfd, F_SETFL, flags|O_NONBLOCK);

    while(1){
        //FD_SET(fileno(stdin), &all);
        //FD_SET(sockfd, &all);
        

        //select((sockfd+1), &all, NULL, NULL, NULL);
        //if(FD_ISSET(sockfd, &all)){
        memset(recvline,'\0',1025);
        read(sockfd, recvline, 1025);
        fputs(recvline, stdout);
        //}

        //if(FD_ISSET(fileno(stdin), &all)){
        memset(inputline,'\0',1025);
        fgets(inputline, 1025, stdin);
        if(strcmp(inputline, "exit\n") == 0){
            close(sockfd);
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
        //sleep end

        //put
        char pch1[3];
        strncpy(pch1, inputline, 3);
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
            int n = write(sockfd, sendline, sizeof(sendline));
            //printf("N: %d\n", n);
            //printf("sendline: %s\n", sendline);


            int pkts = (size / 1025);
            int lastpktsize = 1025;
            if(size % 1025 != 0){ 
                pkts++;
                lastpktsize = size % 1025;
                //printf("lastpktsize: %d\n", lastpktsize);
            }

                //printf("num of pkts: %d\n", pkts);
            int i = 0;
            memset(sendline, '\0', sizeof(sendline));
            fread(sendline, 1, 1025, fp);
            barnum = 0;
            while(i < pkts){
                /*memset(sendline, '\0', sizeof(sendline));
                fread(sendline, 1, 1025, fp);*/
                //int pktsize = min(strlen(sendline), 1025);
                int n = write(sockfd, sendline, sizeof(sendline));
                if(n > 0){
                    /*if(errno == EAGAIN &&){ 
                        printf("EAGAIN! %d %d %d %s\n", errno, n, i, sendline);
                        sleep(1);
                        continue;
                    }*/
                    i++;
                    //FILE *fp2 = fopen("wr", "a+");
                    if(i == pkts){
                        //fwrite(sendline, 1, lastpktsize, fp2);
                        //fclose(fp2);
                        fprintf(stdout, "Progress : [######################]");
                        fflush(stdout);
                        printf("\n");
                        //printf("%s\n", sendline);
                        break;
                    }
                    else{
                        //fwrite(sendline, 1, sizeof(sendline), fp2);
                        //fclose(fp2);
                        /*float barcheck = (float)i / (float)pkts;
                        //printf("i: %d barcheck: %f\n", i, barcheck);
                        if(barcheck *22 > (float)barnum){
                            fprintf(stdout, "Progress : [");
                            fflush(stdout);
                            barnum++;
                            for(int f = 0; f < 22; f++){
                                if((float)f <= barcheck *22){
                                    fprintf(stdout, "#");
                                    fflush(stdout);
                                }
                                else{
                                    fprintf(stdout, " ");
                                    fflush(stdout);
                                }
                            }
                            fprintf(stdout, "]");
                            fflush(stdout);
                            printf("\r");
                        }*/
                        bar(i, pkts, &barnum);
                        //printf("barnum: %d\n", barnum);
                        //printf("%s\n", sendline);
                    }
                }
                //sleep(1);
                memset(sendline, '\0', sizeof(sendline));
                fread(sendline, 1, 1025, fp);
            }


            fclose(fp);
            printf("[Upload] %s Finish!\n", pch2);
            continue;
        }
        //put end

        //receive
        if(recvline[0] == 'p' && recvline[1] == 'u' && recvline[2] == 't'){
            barnum = 0;
            char* pch = strtok(recvline, " ");
            pch = strtok(NULL, " ");
            char filename[1025];
            strcpy(filename, pch);
            //printf("fliename: %s\n", client[i].filename);
            pch = strtok(NULL, " ");
            //printf("size: %s\n", pch);
            int size = atoi(pch);
            //printf("size: %d\n", size);
            int pkts = (size / 1025);
            lastpkt = 1025;
            if(size % 1025 != 0){ 
                pkts++;
                lastpkt = size % 1025;
                //printf("lastpktsize: %d\n", client[i].lastpkt);
            }

            //printf("receive start with size:%d  lastpkt:%d  pkts:%d\n", size, lastpkt, pkts);
            printf("[Download] %s Start!\n", filename);
            FILE *fp = fopen(filename, "ab+");

            int i = 0;     
            while(i < pkts){
                memset(recvline, '\0', sizeof(recvline));
                int n = read(sockfd, recvline, 1025);
                //DIR *dir = opendir(client[i].user_name);
                //int pktsize = min(strlen(buffer), 1025);
                if(n > 0){
                    i++;
                    //printf("recpkt: %d\n", i);   
                    //printf("Use dir:%s\n", usedir);

                    if(i == pkts){
                        fprintf(stdout, "Progress : [######################]");
                        fflush(stdout);
                        printf("\n");
                        fwrite(recvline, 1, lastpkt, fp);      
                        break;          
                    }
                    else{
                        bar(i, pkts, &barnum);
                        fwrite(recvline, 1, sizeof(recvline), fp);
                    }
                }
                    
            }
            fclose(fp);
            printf("[Download] %s Finish!\n", filename);
        }
    //}

    }
    return 0;
}
