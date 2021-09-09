#ifndef PROGETTO_HASHTABLE_H
#define PROGETTO_HASHTABLE_H
#include "list.h"
#include "my_string.h"
#include <pthread.h>
#include <stdbool.h>

typedef struct hashtable{
    unsigned int max_size;
    int collisions;
    pthread_mutex_t lock;
    int length;
    list** buckets;
}hashtable;

int hashtable_insert(hashtable** table, char* key, void* value);
void hashtable_getValue(hashtable* table, char* key);
hashtable* hashtable_create(unsigned int size);
void hashtable_destroy(hashtable** table, void(*delete_value)(void* value));
void hashtable_iterate(hashtable* table, void(*f)(char*, void*, bool*, void*), void* args);
void hashtable_iteraten(hashtable* table, void(*f)(char*, void*, bool*, void*), void* args, int n);
bool hashtable_containsKey(hashtable* table, char* key);
int hashtable_deleteKey(hashtable** table, char* key, void(*delete_value)(void* value));
int hashtable_updateValue(hashtable** table, char* key, void* newValue, void(*delete_value)(void* value));
bool hashtable_isEmpty(hashtable* table);
#endif //PROGETTO_HASHTABLE_H
