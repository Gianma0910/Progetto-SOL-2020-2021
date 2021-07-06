#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "../lib/my_string.h"
#include "../lib/list1.h"

list1 *list1_create(){
    list1 *l = malloc(sizeof(list));
    if(l == NULL){
        fprintf(stderr, "Impossible to allocate the list\n");
        exit(errno);
    }
    l->head = NULL;
    l->tail = NULL;
    l->length = 0;

    return l;
}

void insert_head(node1 **head, char *key, void *value){
    node1 *element = (node1 *) malloc(sizeof(node1));
    if(element == NULL){
        fprintf(stderr, "Impossible create a new node\n");
        exit(errno);
    }
    element->key = key;
    element->value = value;
    element->next = *head;

    *head = element;
}

void insert_tail(node1 **tail, char *key, void *value){
    node1 *element = (node1 *) malloc(sizeof(node1));

    if(element == NULL){
        fprintf(stderr, "Impossible create a new node\n");
        exit(errno);
    }
    element->key = key;
    element->value = value;
    element->next = NULL;

    (*tail)->next = element;
    *tail = element;
}

bool list1_remove(list1 **l, char *key, void (*delete_value)(void *value)){
    node1 **head = &(*l)->head;

    node1 *curr = *head;
    node1 *succ = (*head)->next;

    if(str_equals(curr->key, key)){
        *head = curr->next;
        free(curr->key);
        if(delete_value != NULL){
            delete_value(curr->value);
        }
        free(curr);
        (*l)->length--;

        if((*l)->head == NULL){
            (*l)->tail = NULL;
        }
        return true;
    }

    if(succ == NULL){
        return false;
    }

    while(!str_equals(succ->key, key) && curr != NULL){
        curr = curr->next;
        succ = succ->next;
        if(succ == NULL)
            break;
    }

    if(curr != NULL && succ == NULL){
        return false;
    }else if(curr != NULL){
        curr->next = succ->next;
        if(succ->next == NULL)
            (*l)->tail = curr;

        free(succ->key);
        if(delete_value != NULL)
            delete_value(succ->value);
        free(succ);
        (*l)->length--;

        return true;
    }

    return false;
}

bool list1_isEmpty(list1 *l){
    return l->length == 0 || l->head == NULL;
}

void list1_destroy(list1 **l, void (*delete_value)(void *value)){
    while((*l)->head != NULL){
        node1 *curr = (*l)->head;
        free(curr->key);
        if(delete_value != NULL)
            delete_value(curr->value);

        (*l)->head = curr->next;
        free(curr);
    }

    free((*l));
}

void list1_insert(list1 **l, char *key, void *value){
    char *dup_key = str_create(key);

    if((*l)->head == NULL){
        insert_head(&(*l)->head, dup_key, value);
        (*l)->tail = (*l)->head;
    }else{
        insert_tail(&(*l)->tail, dup_key, value);
    }

    (*l)->length++;
}

node1* list1_getNode(list1 *l, char *key){
    node1 *head = l->head;
    node1 *tail = l->tail;

    if(head == NULL){
        return NULL;
    }

    if(str_equals(head->key, key)){
        return head;
    }else if(str_equals(tail->key, key)){
        return tail;
    }

    while(head != NULL && !str_equals(head->key, key)){
        head = head->next;
    }

    if(head != NULL){
        return head;
    }

    return NULL;
}

int list1_getLength(list1 *l){
    return l->length;
}

bool list1_containsKey(list1 *l, char *key){
    return list1_getNode(l, key) != NULL;
}