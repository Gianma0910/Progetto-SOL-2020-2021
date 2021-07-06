typedef struct node {
    char* pid_client;
    struct node *next;
} node;

typedef struct list2 {
    node *head;
    node *tail;
    int length;
} list2;

#ifndef HASH_TABLE_LIST2_H
#define HASH_TABLE_LIST2_H
#include <stdbool.h>

list2 *list2_create();
bool list2_remove(list2 **l, char* pid_client);
bool list2_isEmpty(list2* l);
node* list2_getNode(list2* l, char* pid_client);
void list2_insert(list2 **l, char* pid_client);
void list2_destroy(list2** l);
bool list2_containsKey(list2* l, char* pid_client);
#endif //HASH_TABLE_LIST2_H
