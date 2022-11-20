#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/stat.h>


#include <pthread.h>
#include "http.h"
#include "fila.h"
#define ROOT "./Arquivos"
#define BUFFER_TAM 60000

void* Thread_Server(void *arg){
    Infos_Threads *request = (Infos_Threads*)(arg);
    char *response = Make_Response(request->request_info, ROOT);
    write(request->newsockfd, response, strlen(response));
    close(request->newsockfd);
    free(request->request_info);
    free(response);
}

int main(int argc, char *argv[]){
    system("clear");

    pthread_t T1;

    Infos_Threads thread;

    char *request = (char*)calloc(BUFFER_TAM, sizeof(char));
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd,5);
    clilen = sizeof(cli_addr);

    int count = 0, fim = 0;
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) error("ERROR on accept");

    while(1){
        n = read(newsockfd, &request[count], 1);
        fim ++;
        if(request[count-1] == '\r' && request[count] == '\n') { // Se encontrar um terminador
            if (fim == 2){ // Se encontrar um terminador de request HTTP

                thread.newsockfd = newsockfd;
                thread.request_info = (char*)calloc(strlen(request), sizeof(char));
                strcpy(thread.request_info, request);
                (void) pthread_create(&T1, NULL, Thread_Server, (void*)(&thread));

                (void) pthread_join(T1,NULL);

                newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
                if (newsockfd < 0) error("ERROR on accept");

                free(request);
                request = (char*)calloc(BUFFER_TAM, sizeof(char));
                count = 0;
                continue;
            }
            fim = 0;
        }
        count++;
        if (n < 0) error("ERROR reading from socket");
    }
    close(sockfd);
    return 0;
}
