char* Make_HTTP_Response(int value, int lenght, char *type, char *buffer, char *last_modified, char *connection_type);

char* HTTP_Version(char *request, int current_id);

char* Make_Response(char *request, char *root);

void error(const char *msg);