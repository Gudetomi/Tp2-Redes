#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


#include <pthread.h>
#include "http.h"
#include "fila.h"
#define ROOT "./Arquivos"
#define BUFFER_TAM 60000
#define BufferSize 10000
#include <pthread.h>
#include <semaphore.h>

sem_t empty;
sem_t full;
int in = 0;
int out = 0;
Infos_Threads buffer[BufferSize];
pthread_mutex_t mutex;

void* producer(void *args){
    char *request = (char*)calloc(BUFFER_TAM, sizeof(char));
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi((char*)args);
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
                sem_wait(&empty);
                pthread_mutex_lock(&mutex);

                buffer[in].newsockfd = newsockfd;
                buffer[in].request_info = (char*)calloc(strlen(request), sizeof(char));
                strcpy(buffer[in].request_info, request);
                buffer[in].used = 1;

                in = (in+1)%BufferSize;
                pthread_mutex_unlock(&mutex);
                sem_post(&full);

                free(request);
                request = (char*)calloc(BUFFER_TAM, sizeof(char));
                count = 0;
                newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
                if (newsockfd < 0) error("ERROR on accept");
                continue;
            }
            fim = 0;
        }
        count++;
        if (n < 0) error("ERROR reading from socket");
    }
    close(sockfd);
}


void* consumer(void *args){
    Infos_Threads request;
    char *response;
    char *request_info;
    int validate = 0;

    while (1) {
        sem_wait(&full);
        pthread_mutex_lock(&mutex);

        if (buffer[out].used) {
            request = buffer[out];
            buffer[out].used = 0;

            request_info = (char*)calloc(strlen(buffer[out].request_info), sizeof(char));
            strcpy(request_info, buffer[out].request_info);
            free(buffer[out].request_info);
            request.request_info = request_info;

            out = (out + 1) % BufferSize;
            validate = 1;
        }
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);

        if (validate){
            response = Make_Response(request.request_info, ROOT);
            write(request.newsockfd, response, strlen(response));
            close(request.newsockfd);
            free(request.request_info);
            free(response);
        }
        validate = 0;
    }
    return NULL;
}


int main(int argc, char *argv[]){

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

//    system("clear");
    int n_threads = 10;

    pthread_t pro, con[n_threads];
    pthread_mutex_init(&mutex, NULL);
    sem_init(&empty,0,BufferSize);
    sem_init(&full,0,0);

    pthread_create(&pro, NULL, (void *)producer, argv[1]);

    for(int i = 0; i < n_threads; i++) {
        pthread_create(&con[i], NULL, (void *)consumer, (void *)NULL);
    }
    for(int i = 0; i < n_threads; i++) {
        pthread_join(con[i], NULL);
    }
    pthread_join(pro, NULL);
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);
    return 0;
}