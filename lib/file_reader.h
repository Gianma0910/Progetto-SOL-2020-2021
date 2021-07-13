#include <stdbool.h>

#ifndef FILE_READER_H
#define FILE_READER_H
char* file_readline(FILE* file);
void* file_readAll(FILE* file);
size_t file_getsize(FILE* file);
bool is_directory(const char *file);
int file_scanAllDir(char*** output, char* init_dir);
int file_nscanAllDir(char*** output, char* init_dir, int n);
#endif
