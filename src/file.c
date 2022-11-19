#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

File_Information Info_Path(char *file_name, char *root){ // Verifica o diretório que o usuário fez a requisição, se é possível encontrá-lo, ou não
    FILE *fp;
    File_Information file_information;
    char diretorio[1024];
    diretorio[0] = '\0';
    strcat(diretorio, root);
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