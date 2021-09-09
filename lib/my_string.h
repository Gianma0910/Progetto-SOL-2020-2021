#include <stdbool.h>
#ifndef PROGETTO_MY_STRING_H
#define PROGETTO_MY_STRING_H

char* str_create(const char* string);
bool str_equals(const char* s1, const char* s2);
bool str_equals_ic(const char* s1, const char* s2);
char* str_concat(const char* s1, const char* s2);
char* str_concatn(const char* s1, ...);
int str_split(char*** output, const char* string, const char* delimiter);
int str_splitn(char*** output, const char* string, const char* delimiter, int n);
bool str_startsWith(const char* string, const char* prefix);
bool str_endsWith(const char* string, const char* suffix);
bool str_isEmpty(const char* string);
char* str_cut(const char* string, int from, int to);
void str_removedNewLine(char** string);
char* str_clean(char* string);
char* str_trim(char* string);
char* str_long_toStr(long n);
int str_length(const char *s);
void str_clearArray(char*** array, const int length);
int str_toInteger(int* output, char* string);
#endif //PROGETTO_MY_STRING_H
