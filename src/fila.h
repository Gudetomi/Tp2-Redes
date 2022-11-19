typedef struct Infos_Threads{
    char *request_info;
    int newsockfd;
}Infos_Threads;

typedef struct Fila{
    Infos_Threads info_thread;
    struct Fila *prox;
}Fila;

typedef struct Descritor{
    Fila *cabeca;
    Fila *ultimo;
}Descritor;

Fila* pop(Descritor *descritor);

Descritor* push(Descritor *descritor, int newsockfd, char *request);
