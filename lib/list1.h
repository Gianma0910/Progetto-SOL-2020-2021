typedef struct node {
    char* key;
    void *value;
    struct node *next;
} node;

typedef struct list1 {
    node *head;
    node *tail;
    int length;
} list1;

#ifndef HASH_TABLE_list1_H
#define HASH_TABLE_list1_H
#include <stdbool.h>

list1 *list_create();
bool list_remove(list1 **l, char* key, void (*delete_value)(void* value));
bool list_isEmpty(list1* l);
node* list_getNode(list1* l, char* key);
void list_insert(list1 **l, char* key, void *value);
void list_destroy(list1** l, void (*delete_value)(void* value));
bool list_containsKey(list1* l, char* key);
#endif //HASH_TABLE_list1_H
