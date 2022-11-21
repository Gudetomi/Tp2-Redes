#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "http.h"
#include "fila.h"
#define ROOT "./Arquivos"
#define BUFFER_TAM 60000

int main(int argc, char *argv[]) {
    system("clear");
    char *request = (char *) calloc(BUFFER_TAM, sizeof(char));
    char *concat_request = (char *) calloc(100000, sizeof(char));
    char *response;
    int sockfd, newsockfd, portno, nbytes, fdmax;
    socklen_t clilen;
    fd_set master;
    fd_set read_fds;
    struct sockaddr_in serv_addr, cli_addr;
    int i, j;
    int yes = 1;
    int count = 0, fim=0;
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    memset(&(serv_addr.sin_zero), '\0', 8);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, 5);

    FD_SET(sockfd, &master);
    fdmax = sockfd;

    while (1) {
        read_fds = master;
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("Server-select() error lol!");
            exit(1);
        }

        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == sockfd) {
                    clilen = sizeof(cli_addr);
                    if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1) {
                        perror("Server-accept() error lol!");
                    } else {
                        FD_SET(newsockfd, &master);
                        if (newsockfd > fdmax) {
                            fdmax = newsockfd;
                        }
                        printf("%s: New connection from %s on socket %d\n", argv[0], inet_ntoa(cli_addr.sin_addr),
                               newsockfd);
                    }
                }
                else {
//                    printf("requisição %s\n", concat_request);
                    if ((nbytes = recv(i, request, 1, 0)) <= 0) {
                        /* got error or connection closed by client */
                        if (nbytes == 0)
                            /* connection closed */
                            printf("%s: socket %d hung up\n", argv[0], i);

                        else
                            perror("recv() error lol!");
                        close(i);
                        FD_CLR(i, &master);
                    }
                    else {
                        strcat(concat_request, request);
//                        printf("requisição %s\n", concat_request);
                        /* we got some data from a client*/
                        /* send to everyone! */
                        if (FD_ISSET(i, &master)) {
                            /* except the sockfd and ourselves */
                            fim ++;
                            if(concat_request[count-1] == '\r' && concat_request[count] == '\n') { // Se encontrar um terminador
                                if (fim == 2){ // Se encontrar um terminador de request HTTP
                                    count++;
                                    concat_request[count] = '\0';
                                    // printf("REQUEST = \n\n%s\n\n", request);
                                    response = Make_Response(concat_request, ROOT);
                                    write(newsockfd, response, strlen(response));
                                    request = (char*)calloc(BUFFER_TAM, sizeof(char));
                                    count = 0;
                                    continue;
                                }
                                fim = 0;
                            }
                            count++;
                        }
//                        free(response);
                    }

                }
            }
        }

    }
}