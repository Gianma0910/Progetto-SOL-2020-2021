#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>
#include "../lib/file_reader.h"
#include "../lib/my_string.h"

char* file_readline(FILE *file){
    static char* line;
    static size_t len;

    int read_lines = (int) getline(&line, &len, file);
    if(read_lines == -1)
        return NULL;

    str_removeNewLine(&line);
    char* ret = str_create(line);
    return ret;
}

void* file_readAll(FILE *file){
    size_t file_size = file_getsize(file);

    void *buffer = malloc(sizeof(char) * file_size);
    if(buffer == NULL){
        fprintf(stderr, "Impossible allocate space: file_readAll() malloc error\n");
        return NULL;
    }
    fread(buffer, sizeof(char), file_size, file);

    return buffer;
}

size_t file_getsize(FILE *file){
    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    return size;
}

void file_close(FILE *fp){
    fclose(fp);
}

bool is_directory(const char *file){
    struct stat p;
    stat(file, &p);
    return S_ISDIR(p.st_mode);
}

int file_scanAllDir(char ***output, char* init_dir){
    return file_nscanAllDir(output, init_dir, -1);
}

int file_nscanAllDir(char ***output, char *init_dir, int n) {
    static int max = 2;
    static int current_lenght = 0;
    static int count = 0;
    count = n;
    DIR *current_dir = opendir(init_dir);

    if (current_dir == NULL || count == 0) {
        return current_lenght;
    }

    if (*output == NULL) {
        *output = calloc(max, sizeof(char *));
    }

    struct dirent *file;

    while ((file = readdir(current_dir)) != NULL) {
        if (count == 0)
            break;

        if (current_lenght == max) {
            max *= 2;
            *output = realloc(*output, max * sizeof(char *));
        }
        char *file_name = file->d_name;

        if (strcmp(".", file_name) != 0 && strcmp("..", file_name) != 0 && file_name[0] != '.') {
            file_name = str_concatn(init_dir, "/", file_name, NULL);
            char *file_path = realpath(file_name, NULL);
            assert(file_path != NULL);

            if (!is_directory(file_path)) {
                (*output)[current_lenght] = file_path;
                free(file_name);
                current_lenght++;
                count--;
            } else {
                current_lenght = file_nscanAllDir(output, file_path, count);
            }
        }
    }

    closedir(current_dir);
    return current_lenght;
}

