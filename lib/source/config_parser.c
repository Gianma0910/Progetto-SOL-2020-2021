#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../config_parser.h"
#include "../my_string.h"
#include "../file_reader.h"
#include <errno.h>
#include <limits.h>
#include "../myerrno.h"

#define CHECK_ULONG_LIMIT(x) if((x)>ULONG_MAX){ \
            (x)=ULONG_MAX; \
        }

#define CHECK_UINT_LIMIT(x) if((x)>UINT_MAX){ \
            (x)=UNIT_MAX; \
        }

size_t convert_str(char* string){
    char* type;
    size_t converted_s = strtol(s, &type, 10);
    type = str_clean(type);

    if(str_equals_ic(type, "m")) return converted_s * 1024 * 1024;
    else if(str_equals_ic(type, "g")) return converted_s * 1024 * 1024 * 1024;
    else if(str_isEmpty(type)) return converted_s;

    errno = ERROR_CONV;
    return -1;
}

void settings_default(Settings* s){
    s->N_THREAD_WORKERS = 5;
    s->MAX_STORAGE_SPACE = 209715200;
    s->MAX_STORABLE_FILES = 100;
    s->SOCK_PATH = str_create("mysock");
    s->PRINT_LOG = 1;
    s->LOG_FILE = str_create("log_file.txt");
}

void setConfigFor(Settings* s, char* key, char* value){
    key = str_clean(key);
    int converted_v = 0;
    size_t sp;

    if(str_equals(key, "N_THREAD_WORKERS")){
        if(str_toInteger(&converted_v, value) != 0){
            fprintf(stderr, "Error on parsing [%s]: default value set\n", key);
            return;
        }

        CHECK_UINT_LIMIT(converted_v);
        s->N_THREAD_WORKERS = converted_v;
    }else if(str_equals(key, "MAX_STORAGE_SPACE")){
        errno = 0;
        sp = convert_str(value);
        if(errno == ERROR_CONV){
            fprintf(stderr, "Error on parsing [%s]: default value set\n", key);
            return;
        }

        CHECK_ULONG_LIMIT(sp);
        s->MAX_STORAGE_SPACE = sp;
    }else if(str_equals(key, "MAX_STORABLE_FILES")){
        if(str_toInteger(&converted_v, value) != 0){
            fprintf(stderr, "Error on parsing [%s]: default value set\n", key);
            return;
        }

        CHECK_UINT_LIMIT(converted_v);

        s->MAX_STORABLE_FILES = converted_v;
    }else if(str_equals(key, "SOCK_PATH")){
        free(s->SOCK_PATH);
        char* new_path = str_clean(value);
        s->SOCK_PATH = str_create(new_path);
    }else if(str_equals(key, "PRINT_LOG")){
        if(str_toInteger(&converted_v, value) != 0){
            fprintf(stderr, "Error on parsing [%s]: default value set\n", key);
            return;
        }
        if(converted_v != 0 && converted_v != 1 && converted_v != 2){
            fprintf(stderr, "PRINT_LOG option must be 0, 1 or 2. See the documentation\n"
                            "Default value set\n");
            return;
        }
        s->PRINT_LOG = converted_v;
    }else if(str_equals(key, "LOG_FILE")){
        free(s->LOG_FILE);
        char* log_file = str_clean(value);
        s->LOG_FILE = str_create(log_file);
    }
}

void settings_load(Settings* s, char* path, char* log){
    FILE *c;
    FILE *file_log;

    if(path == NULL || str_isEmpty(path)){
        c = fopen("config.ini", "r");
    }else{
        c = fopen(path, "r");
    }
    if(c == NULL){
        settings_default(s);
        return;
    }

    if(log == NULL || str_isEmpty(log)){
        file_log = fopen("log_file.txt", "w");
    }else{
        file_log = fopen(log, "w");
    }
    if(file_log == NULL){
        settings_default(s);
        return;
    }

    if(s->SOCK_PATH == NULL){
        s->SOCK_PATH = str_create("mysock");
    }
    if(s->LOG_FILE == NULL){
        s->LOG_FILE = str_create("log_file.txt");
    }

    char** array = NULL;
    char* line;
    while((line = file_readLine(c)) != NULL){
        char* cleaned_line = str_clean(line);
        if(!str_startsWith(cleaned_line, "#") && !str_isEmpty(cleaned_line)){
            int n = str_splitn(&array, cleaned_line, "=#", 3);
            setConfigFor(s, array[0], array[1]);
            str_clearArray(&array, n);
        }
        free(line);
    }
}

void settings_free(Settings* s){
    free(s->SOCK_PATH);
    free(s->LOG_FILE);
}

void settings_print(Settings s){
    enum Color c = MAGENTA;

    pcolor(c, "MAX_STORABLE_FILES:\t\t\t");
    printf("%u\n", s.MAX_STORABLE_FILES);

    pcolor(c, "MAX_STORAGE_SPACE:\t\t\t");
    printf("%lu\n", s.MAX_STORAGE_SPACE);

    pcolor(c, "N_THREAD_WORKERS:\t\t\t");
    printf("%u\n", s.N_THREAD_WORKERS);

    pcolor(c, "SOCK_PATH:\t\t\t");
    printf("%s\n", s.SOCK_PATH);

    pcolor(c, "PRINT_LOG:\t\t\t");
    printf("%d\n", s.PRINT_LOG);

    pcolor(c, "LOG_FILE:\t\t\t");
    printf("%s\n", s.LOG_FILE);

    pwarn("");
    pcode(0, NULL);
    psucc("");
}