typedef struct{
    size_t MAX_STORAGE_SPACE;
    unsigned int N_THREAD_WORKERS;
    unsigned int MAX_STORABLE_FILES;
    int PRINT_LOG;
    char* SOCK_PATH;
} settings;

#ifndef CONFIG_PARSER_CONFIG_PARSER_H
#define CONFIG_PARSER_CONFIG_PARSER_H
void settings_free(settings* s);
void settings_load(settings* s, char* path);
void settings_default(settings* s);
void settings_print(settings s);
#endif //CONFIG_PARSER_CONFIG_PARSER_H
