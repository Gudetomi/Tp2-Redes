#include "requests.h"
#include <stdlib.h>
#include <string.h>

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
