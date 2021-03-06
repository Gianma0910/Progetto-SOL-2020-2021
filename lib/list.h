typedef struct node{
    char* key;
    void* value;
    struct node* next;
}node;

typedef struct list{
    node* head;
    node* tail;
    int length;
}list;

#ifndef PROGETTO_LIST_H
#define PROGETTO_LIST_H
#include <stdbool.h>

list* list_create();
bool list_remove(list** l, char* key, void(*delete_value)(void* value));
bool is_Empty(list* l);
node* list_getNode(list* l, char* key);
void list_insert(list** l, char* key, void* value);
void list_destroy(list** l, void(*delete_value)(void* value));
bool list_containsKey(list* l, char* key);
#endif //PROGETTO_LIST_H
