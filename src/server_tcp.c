#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "http.h"
#include "fila.h"
#define ROOT "./Arquivos"
#define BUFFER_TAM 60000

int main(int argc, char *argv[]){
    system("clear");
    char *request = (char*)calloc(BUFFER_TAM, sizeof(char));
    char *response;
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
                count++;
                request[count] = '\0';
                // printf("REQUEST = \n\n%s\n\n", request);
                response = Make_Response(request, ROOT);
                write(newsockfd, response, strlen(response));
                free(response);
                close(newsockfd);
                // printf("\n\n_______________________________________________________________________________________________________________________\n\n");
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