#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Fila{
    Infos_Threads info_thread;
    struct Fila *prox;
}Fila;

typedef struct Infos_Threads{
    char *request_info;
    int newsockfd;
} Infos_Threads;

typedef struct Descritor{
    Fila *cabeca;
    Fila *ultimo;
}Descritor;


Descritor* Move_Fila(Descritor descritor){ // Move a cabeça da fila para a proxima celula
    Fila *aux;
    aux = descritor.cabeca;
    if (descritor.cabeca.prox != NULL){
        descritor.cabeca = descritor.cabeca.prox;
    }
    free(aux);
    return descritor;
}

Descritor* Insere_fila(Descritor *descritor, int newsockfd, char *request){ // Insere uma requisição na fila
    if (descritor == NULL){ // Descritor vazio
        descritor = (Descritor*)calloc(1, sizeof(Descritor));
        descritor->cabeca = NULL;
        descritor->ultimo = NULL;
    }
    
    if (descritor->cabeca == NULL){ // Fila vazia
        descritor->cabeca = (Fila*)calloc(1, sizeof(Fila));
        descritor->cabeca.info_thread.newsockfd = newsockfd; // Armazena informações da requisição
        descritor->cabeca->info_thread.request_info = (char*)calloc(strlen(request), sizeof(char));
        strcpy(descritor->cabeca.info_thread.request_info, request); // Armazena informações da requisição
        descritor->cabeca->prox = NULL;
        descritor->ultimo = descritor->cabeca; // Inicio é o mesmo do fim
        return descritor;
    }else{ // Fila não vazia
        descritor->ultimo->prox = (Fila*)calloc(1, sizeof(Fila));
        descritor->ultimo = descritor->ultimo->prox;
        descritor->ultimo->info_thread.newsockfd = newsockfd;
        descritor->ultimo->info_thread.request_info = (char*)calloc(strlen(request), sizeof(char));
        strcpy(descritor->ultimo->info_thread.request_info, request);
        descritor->ultimo->prox = NULL;
    }
    return descritor;
}


void* Thread_Server(void *arg){
    Infos_Threads *request = (Infos_Threads*)(arg);
    char *response = Make_Response(request->request_info);	
    write(request->newsockfd, response, strlen(response));
    close(request->newsockfd);
    free(request->request_info);
    free(response);
}

int main(int argc, char *argv[]){
    system("clear");

    pthread_t T1;

    Descritor *descritor;

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

                /* TODO: Verificar se tem thread para atender Multithread */
                descritor = Insere_fila(descritor, newsockfd, request); // Descritor para atender a FILA


                
                // if (Tem_Thread_Para_Atender){
                    (void) pthread_create(&T1, NULL, Thread_Server, (void*)(&descritor->cabeca->info_thread)); // Single thread
                    (void) pthread_join(T1,NULL); // Single Thread

                    descritor = Move_Fila(descritor);
                // }
               
                /* FIM TODO....*/


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


