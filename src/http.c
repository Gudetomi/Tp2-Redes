#include "http.h"
#include "file.h"
#include "requests.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NOT_IS_GET (strcmp(verify, "GET") != 0)
#define IS_POST (strcmp(verify, "POST") == 0)
#define IS_HEAD (strcmp(verify, "HEAD") == 0)
#define IS_PUT (strcmp(verify, "PUT") == 0)
#define IS_DELETE (strcmp(verify, "DELETE") == 0)
#define IS_CONNECT (strcmp(verify, "CONNECT") == 0)
#define IS_OPTION (strcmp(verify, "OPTION") == 0)
#define IS_TRACE (strcmp(verify, "TRACE") == 0)
#define IS_PATCH (strcmp(verify, "PATCH") == 0)


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
    return response;
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

char* Make_Response(char *request, char *root){
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
                file_information = Info_Path(diretorio, root);
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

void error(const char *msg){
    perror(msg);
    exit(1);
}