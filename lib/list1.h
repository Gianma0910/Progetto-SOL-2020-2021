typedef struct node1 {
    char* key;
    void *value;
    struct node1 *next;
} node1;

typedef struct list1 {
    node1 *head;
    node1 *tail;
    int length;
} list1;

#ifndef HASH_TABLE_list1_H
#define HASH_TABLE_list1_H
#include <stdbool.h>

list1 *list1_create();
bool list1_remove(list1 **l, char* key, void (*delete_value)(void* value));
bool list1_isEmpty(list1* l);
node1* list1_getNode(list1* l, char* key);
void list1_insert(list1 **l, char* key, void *value);
void list1_destroy(list1** l, void (*delete_value)(void* value));
bool list1_containsKey(list1* l, char* key);
#endif //HASH_TABLE_list1_H
