#define DEFAULT_SETTINGS {.N_THREAD_WORKERS=5, .MAX_STORAGE_SPACE=209715200, .MAX_STORABLE_FILES=100, .SOCK_PATH=null, .PRINT_LOG=1, .LOG_FILE="log_file.txt"};
#define ERROR_CONV 1

typedef struct Settings{
       size_t MAX_STORAGE_SPACE;
       unsigned int N_THREAD_WORKERS;
       unsigned int MAX_STORABLE_FILES;
       int PRINT_LOG;
       char* SOCK_PATH;
       char* LOG_FILE;
}Settings;

#ifndef PROGETTO_CONFIG_PARSER_H
#define PROGETTO_CONFIG_PARSER_H
void settings_free(Settings* s);
void settings_load(Settings* s, char* path, char* log);
void settings_default(Settings* s);
void settings_print(Settings s);
#endif //PROGETTO_CONFIG_PARSER_H
