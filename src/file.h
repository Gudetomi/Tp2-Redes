#include <sys/stat.h>
#include <time.h>

typedef struct{
    char *content;
    long int size;
    int code;
    char *type;
    char *last_modified;
    char *connection_type;
}File_Information;

File_Information Take_File_Information(File_Information file_information, char *diretorio);

File_Information Info_Path(char *file_name, char *root);

char *Found_File_Name(char *request, int current_id);

char *Last_Modified(char *path);