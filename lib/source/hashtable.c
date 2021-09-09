#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include "../hashtable.h"
#include "../myerrno.h"

pthread_mutex_t table_lock = PTHREAD_MUTEX_INITIALIZER;

int hash(char* str, int t_size){
    int hash, i;
    int len = str_length(str);

    for(hash = i = 0; i < len; i++){
        hash += str[i];
        hash += (hash << 10);
        hash ^= (hash >> 6)
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    hash %= t_size;
    if(hash < 0)
        hash += t_size;

    return hash;
}

pthread_mutex_t table_lock2 = PTHREAD_MUTEX_INITIALIZER;

bool containsKey(hashtable table, char* key, int index){
    pthread_mutex_lock(&table_lock2);
    if(index < 0){
        index = hash(key, (int)table.max_size);
    }
    if(table.buckets[index] == NULL){
        pthread_mutex_unlock(&table_lock2);
        return false;
    }

    if(list_getNode(table.buckets[index], key) != NULL){
        pthread_mutex_unlock(&table_lock2);
        return true;
    }
    pthread_mutex_unlock(&table_lock2);
    return false;
}

bool hashtable_containsKey(hashtable* table, char* key){
    pthread_mutex_lock(&table_lock);
    bool condition = containsKey((*table), key, -1);
    pthread_mutex_unlock(&table_lock);
    return condition;
}

int hashtable_insert(hashtable** table, char* key, void* value){
    if(table == NULL || key == NULL){
        return HASH_NULL_PARAM;
    }

    pthread_mutex_lock(&table_lock);
    int index = hash(key, (*table)->max_size);

    if((*table)->buckets[index] == NULL){
        (*table)->buckets[index] = list_create();
    }else{
        if(containsKey(*(*table), key, index)){
            pthread_mutex_unlock(&table_lock);
            return HASH_DUPLICATE_KEY;
        }

        (*table)->collisions++;
    }

    list_insert(&(*table)->buckets[index], key, value);
    (*table)->length++;

    pthread_mutex_unlock(&table_lock);

    return HASH_INSERT_SUCCESS;
}

int hashtable_updateValue(hashtable** table, char* key, void* newValue, void(*delete_value)(void*)){
    pthread_mutex_lock(&(*table)->lock);
    int index = hash(key, (*table)->max_size);
    if((*table)->buckets[index] == NULL){
        pthread_mutex_unlock(&(*table)->lock);
        return -1;
    }else{
        node* n = list_getNode((*table)->buckets[index], key);
        if(n == NULL){
            pthread_mutex_unlock(&(*table)->lock);
            return -1;
        }else{
            if(delete_value != NULL)
                delete_value(n->value);
            n->value = newValue;
        }
    }

    pthread_mutex_unlock(&(*table)->lock);
    return 0;
}

void* hashtable_getValue(hashtable* table, char* key){
    pthread_mutex_lock(&table_lock);
    int index = hash(key, table->max_size);
    if(table->buckets[index] == NULL){
        pthread_mutex_unlock(&table_lock);
        return NULL;
    }else{
        pthread_mutex_unlock(&table_lock);
        return list_getNode(table->buckets[index], key)->value;
    }
}

hashtable* hashtable_create(unsigned int size){
    list** buckets = calloc(size, sizeof(list*));

    if(buckets == NULL){
        fprintf(stderr, "Error in creation of the hashtable: out of memory");
        exit(-1);
    }
    hashtable *table = malloc(sizeof(hashtable));
    if(table == NULL){
        perr("Impossible to create the hashtable, malloc error\n");
        exit(errno);
    }

    table->max_size = size;
    table->buckets = buckets;
    table->collisions = 0;
    table->length = 0;

    if(pthread_mutex_init(&table->lock, NULL) != 0){
        fprintf(stderr, "Impossible to make the table atomic\n");
    }
    return table;
}

void hashtable_destroy(hashtable** table, void(*delete_value)(void*)){
    if(delete_value != NULL){
        for(int i = 0; i < (*table)->max_size; i++){
            if((*table)->buckets[i] != NULL){
                list_destroy(&(*table)->buckets[i], delete_value);
            }
        }
    }
    free((*table)->buckets);
    free(*table);

    pwarn("");
    pcode(0, NULL);
    psucc("");
    pcolor(STANDARD, "");
}

void hashtable_iterate(hashtable* table, void (*f)(char*, void*, bool*, void*), void* args){
    pthread_mutex_lock(&(table)->lock);
    bool exit = false;
    for(int i = 0; i < table->max_size; i++){
        if(table->buckets[i] != NULL){
            node* head = table->buckets[i]->head;
            while(head != NULL && !exit){
                f(head->key, head->value, &exit, args);
                head = head->nex;
            }
        }

        if(exit)
            break;
    }

    pthread_mutex_unlock(&(table)->lock);
}

bool hashtable_isEmpty(hashtable* table){
    return table->length == 0;
}

void hashtable_iteraten(hashtable* table, void(*f)(char*, void*, bool*, void*), void* args, int n){
    if(n > table->max_size || n <= 0){
        hashtable_iterate(table, f, args);
        return;
    }
    pthread_mutex_lock(&(table)->lock);
    int i = 0;
    bool exit = false;
    while(n > 0 && i < table->max_size){
        if(table->buckets[i] != NULL){
            node *head = table->buckets[i]->head;
            while(head != NULL && n > 0 && !exit){
                f(head->key, head->value, &exit, args);
                head = head->next;
                n--;
            }
        }
        i++;
        if(exit)
            break;
    }
    pthread_mutex_unlock(&(table)->lock);
}

int hashtable_deleteKey(hashtable** table, char* key, void(*delete_value)(void*)){
    pthread_mutex_lock(&(*table)->lock);
    int index = hash(key, (*table)->max_size);

    if((*table)->buckets[index] == NULL){
        pthread_mutex_unlock(&(*table)->lock);
        return HASH_KEY_NOT_FOUND;
    }else{
        bool deleted = list_remove(&(*table)->buckets[index], key, delete_value);
        if(is_Empty((*table)->buckets[index])){
            free((*table)->buckets[index]);
            (*table)->buckets[index] = NULL;
        }
        if(!deleted){
            pthread_mutex_unlock(&(*table)->lock);
            return HASH_KEY_NOT_FOUND;
        }
        (*table)->length--;
    }
    pthread_mutex_unlock(&(*table)->lock);
    return true;
}