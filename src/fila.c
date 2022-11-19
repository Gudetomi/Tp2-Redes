#include "fila.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Fila* pop(Descritor *descritor){ // Move a cabeça da fila para a proxima celula
    Fila *aux;
    if (descritor->cabeca->prox != NULL){
        aux = descritor->cabeca;
        descritor->cabeca = descritor->cabeca->prox;
    }else{
        aux = descritor->cabeca;
        descritor->cabeca = NULL;
    }
    return aux;
}

Descritor* push(Descritor *descritor, int newsockfd, char *request){ // Insere uma requisição na fila
    if (descritor == NULL){ // Descritor vazio
        descritor = (Descritor*)calloc(1, sizeof(Descritor));
        descritor->cabeca = NULL;
        descritor->ultimo = NULL;
    }

    if (descritor->cabeca == NULL){ // Fila vazia
        descritor->cabeca = (Fila*)calloc(1, sizeof(Fila));
        descritor->cabeca->info_thread.newsockfd = newsockfd; // Armazena informações da requisição
        descritor->cabeca->info_thread.request_info = (char*)calloc(strlen(request), sizeof(char));
        strcpy(descritor->cabeca->info_thread.request_info, request); // Armazena informações da requisição
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
    printf("\n\n");
    return descritor;
}

