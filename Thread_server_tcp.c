/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
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


#define ROOT "./Arquivos"
#define BUFFER_TAM 60000


#define NOT_IS_GET (strcmp(verify, "GET") != 0)
#define IS_POST (strcmp(verify, "POST") == 0)
#define IS_HEAD (strcmp(verify, "HEAD") == 0)
#define IS_PUT (strcmp(verify, "PUT") == 0)
#define IS_DELETE (strcmp(verify, "DELETE") == 0)
#define IS_CONNECT (strcmp(verify, "CONNECT") == 0)
#define IS_OPTION (strcmp(verify, "OPTION") == 0)
#define IS_TRACE (strcmp(verify, "TRACE") == 0)
#define IS_PATCH (strcmp(verify, "PATCH") == 0)

typedef struct{
    char *content;
    long int size;
    int code;
    char *type;
    char *last_modified;
    char *connection_type;
}File_Information;

typedef struct Infos_Threads{
    char *request_info;
    int newsockfd;
} Infos_Threads;


/* ________________________________________________________________ TREAT RESOPNSE ________________________________________________________________*/

char* Make_HTTP_Response(int value, int lenght, char *type, char *buffer, char *last_modified, char *connection_type){
    int resposta = 0;
    char integers1[10], integers2[10];
    char *response;
    response = (char*)calloc(100000,sizeof(char));
    strcat(response, "HTTP/1.1 "); // Versão do HTTP
    sprintf(integers1,"%d",value); // Qual o código da resposta
    strcat(response, integers1);

    if (value == 200){ //Arquivo encontrado e será enviado
        strcat(response, " OK\r\n");
        resposta = 1;
    }else if (value == 304){ // Arquivo não foi modificado desde a última visita
        strcat(response, " Not Modified\r\n");
    }else if(value == 400){ // Se a sintaxe da requisição não for correta
        strcat(response, " Bad Request\r\n");
    }else if(value == 404){  // Não encontrou o arquivo
        strcat(response, " Not Found\r\n");
    }else if(value == 501){  // Servidor só aceita GET, qualquer outra requisição não foi implementada
        strcat(response, " Not Implemented\r\n");
    }else if (value == 505){ // Versão do HTTP diferente de 1.1
        strcat(response, " HTTP Version Not Supported\r\n");
    }

    if (resposta == 1 && last_modified != NULL){ // Se for uma requisição com reposta 200, informar a última modificação do arquivo
        strcat(response, "Last-Modified: ");
        strcat(response, last_modified);
        response[strlen(response)-1] = '\r';
        response[strlen(response)] = '\n';
    }
    strcat(response, "Content-Length: "); // Tamanho da resposta
    sprintf(integers2, "%d", lenght);
    strcat(response, integers2);
    strcat(response, "\r\n");

    strcat(response, "Content-Type: "); // Qual o tipo de retorno (maioria será "text/html")
    strcat(response, type);
    strcat(response, "\r\n");

    if (connection_type != NULL){ // Se o servidor especificar como será a conexão
        strcat(response, "Connection: "); // Fecha a conexãp (Closed), ou mantém ela persistente (Keep-Alive)
        strcat(response, connection_type);
        strcat(response, "\r\n");
    }else{ // Se o servidor não especificar, padrão = CLOSED
        strcat(response, "Connection: Closed\r\n");
    }
    
    strcat(response, "\r\n");
    if(buffer != NULL){ // Se houver conteúdo para ser mostrado para o usuário
        strcat(response, buffer);
    }

    response[strlen(response)-1] = '\0'; // Coloca um terminador de string no final

    // printf("\n\nResposta: \n\n%s\n",response);
	return response;
}



char *Last_Modified(char *path){
    struct stat attr;
    stat(path, &attr);
    return ctime(&attr.st_mtime);
}

File_Information Take_File_Information(File_Information file_information, char *diretorio){ // Pega o conteúdo e o tamanho do arquivo
    FILE *fp;
    char buffer[1024];
    int size = 0;
    fp = fopen(diretorio, "r"); // Arquivo com HTML de Not Found
    while (!feof(fp)){ // Encontrar o tamanho do arquivo e seu conteudo
        buffer[size] = fgetc(fp);
        size++;
    }
    fclose(fp);

    file_information.content = (char*)calloc(size, sizeof(char));
    strcat(file_information.content, buffer);
    file_information.size = size-1;
    file_information.type = (char*)calloc(9, sizeof(char));
    strcat(file_information.type, "text/html");

    return file_information;
}

/* ________________________________________________________________ TREAT REQUEST ________________________________________________________________*/

File_Information Info_Path(char *file_name){ // Verifica o diretório que o usuário fez a requisição, se é possível encontrá-lo, ou não
    FILE *fp;
    File_Information file_information;
    char diretorio[1024];
    diretorio[0] = '\0';    
    strcat(diretorio, ROOT);
    strcat(diretorio, file_name);
    fp = fopen(diretorio, "r");
    if (fp == NULL){ // Se o diretorio pesquisado pelo usuario não existe
        bzero(diretorio, 1024);
        strcat(diretorio, "./response_errors_body/not_found.html");
        file_information = Take_File_Information(file_information, diretorio); // Error not found
        file_information.code = 404;
        file_information.last_modified = NULL;


    }else{ // Se há um arquivo para o diretório
        file_information = Take_File_Information(file_information, diretorio);
        file_information.code = 200;
        file_information.last_modified = (char*)calloc(150, sizeof(char));
        strcat(file_information.last_modified, Last_Modified(diretorio));
        fclose(fp);
    }
    return file_information;
}

char* HTTP_Version(char *request, int current_id){ // Encontrar a versao do HTTP (servidor só aceita 1.1)
    int i, cont = 0, j = 0;
    char *http_version;
    for(i = current_id; request[i] != '\r' && request[i+1] != '\n'; i++){ // Tamanho da string com a versão do HTTP
        cont++;
    }
    http_version = (char*)calloc(cont, sizeof(char));
    for(i = current_id; j < cont; i++){ // Armazena qual a versão do HTTP da requisição
        http_version[j] = request[i];
        j++;
    }
    return http_version;
}

char *Found_File_Name(char *request, int current_id){ // Encontrar qual diretório do servidor o usuário quer abrir
    int i, cont = 0, j = 0;
    char *diretorio;
    for(i = current_id; request[i] != 32; i++){ // Espaço (diretório/URL não tem espaço)
        cont ++;
    }
    diretorio = (char*)calloc(cont, sizeof(char));
    for(i = current_id; j < cont; i++){
        diretorio[j] = request[i];
        j++;
    }
    return diretorio;
}

char* Verify_Request_Get(char *request){ // Se a requisição é GET
    int i, size = 0;
    char *verify;
    for(i = 0; request[i] != 32; i++){
        size++;
    }
    verify = (char*)calloc(size, sizeof(char));
    for(i = 0; request[i] != 32; i++){
        verify[i] = request[i];
    }
    return verify;
}

int Verify_Cache_Control(char *request, int valor){ // Verifica se a requisição faz Cache Control, ou seja, se é uma repetição da requisição
    int i = 0, j = 0, verificador = 0;
    char verify[1024];
    for (i = valor; i < strlen(request); i++){
        if (request[i] == ':'){
            verify[j] = '\0';
            if(strcmp(verify, "Cache-Control") == 0){
                verificador = 1;
                break;
            }
        }else{
            verify[j] = request[i];
        }
        j++;
        if(request[i] == '\n'){
            j = 0;
        }            
    }
    return verificador;    
}

char* Verify_Connection(char *request, int valor){ // Verifica o tipo de conexão (Keep-alive ou Closed)
    int i = 0, j = 0, k = 0, verificador = 0;
    char verify[1024];
    char *connection_type = NULL;
    for(i = valor; i < strlen(request); i++){
        if(request[i] == ':'){
            verify[j] = '\0';
            if (strcmp(verify, "Connection") == 0){ // Se na requisição o servidor especificar como ficará a conexão
                verificador = 1;
                connection_type = (char*)calloc(10, sizeof(char));
            }
            if (verificador == 1){ // Verificar qual é o tipo de conexão que o servidor especificou na requisição
                for(i = i + 2; request[i] != '\r' && request[i+1] != '\n'; i++){
                    connection_type[k] = request[i];
                    k++;
                }
                verificador = 0;
                break;
            }
        }else{
            verify[j] = request[i];
        }
        j++;
        if(request[i] == '\n'){
            j = 0;
        }
    }
    return connection_type;
}


/* ________________________________________________________________ PRINCIPAL ________________________________________________________________*/
char* Make_Response(char *request){
    char *diretorio;
    char *verify;
    char *http_version;
    char *connection_type = NULL;
    int verify_cache = 0;
    int valor;
    File_Information file_information;
    verify = Verify_Request_Get(request); // Se for diferente de GET, internal server error
    if (NOT_IS_GET && (IS_POST || IS_HEAD || IS_PUT || IS_DELETE || IS_CONNECT || IS_OPTION || IS_TRACE || IS_PATCH)){ // Verifica se foi feito realmente um GET ou outra requisição
        diretorio = (char*)calloc(44, sizeof(char));
        strcat(diretorio, "./response_errors_body/not_implemented.html");
        file_information = Take_File_Information(file_information, diretorio);
        file_information.code = 501;
        file_information.last_modified = NULL;
        file_information.connection_type = NULL;

    }else if(NOT_IS_GET){ // Se não for nenhum dos outros métodos e não for GET, então é uma requisição errada EX: (PAST // GAT, coisas que não existem)
        diretorio = (char*)calloc(40, sizeof(char));
        strcat(diretorio, "./response_errors_body/bad_request.html");
        file_information = Take_File_Information(file_information, diretorio);
        file_information.code = 400;
        file_information.last_modified = NULL;
        file_information.connection_type = NULL;

    }else{ // Se for uma requisição do tipo GET
        diretorio = Found_File_Name(request, strlen(verify) + 1); // Pega o nome do arquivo
        if(strcmp(diretorio, "/") == 0){
            free(diretorio);
            diretorio = (char*)calloc(40, sizeof(char));
            strcat(diretorio, "./response_errors_body/bad_request.html");
            file_information = Take_File_Information(file_information, diretorio);
            file_information.code = 400;
            file_information.last_modified = NULL;
            file_information.connection_type = NULL;     
        }else{
            http_version = HTTP_Version(request, strlen(verify) + strlen(diretorio) + 2); // Verifica a versão do HTTP (se for diferente de HTTP/1.1, erro) = Erro 505
            valor = strlen(verify) + strlen(diretorio) + strlen(http_version) + 4;
            connection_type = Verify_Connection(request, valor);

            if (strcmp(http_version, "HTTP/1.1") != 0){ // Server só aceita HTTP 1.1
                bzero(diretorio, 1024);
                strcat(diretorio, "./response_errors_body/http_not_supported.html");
                file_information = Take_File_Information(file_information, diretorio);
                file_information.code = 505;
                file_information.last_modified = NULL;
            }else{
                verify_cache = Verify_Cache_Control(request, valor); // Verifica se a requisição é repetida, ou não
                if (verify_cache == 1){
                    file_information.code = 304;
                    file_information.last_modified = NULL;
                    file_information.content = NULL;
                }
                file_information = Info_Path(diretorio);
            }
        }
    }

    if(connection_type != NULL){
        file_information.connection_type = (char*)calloc(strlen(connection_type), sizeof(char));
        strcat(file_information.connection_type, connection_type);
    }else{
        file_information.connection_type = NULL;
    }
   	return Make_HTTP_Response(file_information.code, file_information.size, file_information.type, file_information.content, file_information.last_modified, file_information.connection_type);
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}


/* ________________________________________________________________  TREAT THREADS ________________________________________________________________*/

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
