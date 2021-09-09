#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "../my_string.h"
#include "../list.h"

list* list_create(){
    list* l = malloc(sizeof(list));
    if(l == NULL){
        fprintf(stderr, "Impossible to create a list, malloc error\n");
        exit(errno);
    }
    l->head = NULL;
    l->tail = NULL;
    l->length = 0;

    return l;
}

void insert_head(node** head, char* key, void* value){
    node* element = (node*) malloc(sizeof(node));
    if(element == NULL){
        fprintf(stderr, "list malloc error: impossible to create a new node\n");
        exit(errno);
    }
    element->key = key;
    element->value = value;
    element->next = *head;

    *head = element;
}

void insert_tail(node** tail, char* key, void* value){
    node* element = (node*) malloc(sizeof(node));
    if(element == NULL){
        fprintf(stderr, "list malloc error: impossible to create a new node\n");
        exit(errno);
    }
    element->key = key;
    element->value = value;
    element->next = NULL;

    (*tail)->next = element;
    *tail = element;
}

bool list_remove(list** l, char* key, void(*delete_value)(void* value)){
    node** head = &(*l)->head;

    node* current = *head;
    node* next_node = (*head)->next;

    if(str_equals(current->key, key)){
        *head = current->next;
        free(current->key);
        if(delete_value != NULL)
            delete_value(current->value);
        free(current);
        (*l)->length--;

        if((*l)->head == NULL){
            (*l)->tail = NULL;
        }
        return true;
    }

    if(next_node == NULL){
        return false;
    }

    while(!str_equals(current->key, key) && current != NULL){
        current = current->next;
        next_node = next_node->next;
        if(next_node == NULL)
            break;
    }

    if(current != NULL && next_node == NULL)
        return false;
    else if(current != NULL){
        current->next = next_node->next;
        if(next_node->next == NULL)
            (*l)->tail = current;

        free(next_node->key);
        if(delete_value != NULL)
            delete_value(next_node->value);
        free(next_node);
        (*l)->length--;
        return true;
    }

    return false;
}

bool is_Empty(list* l){
    return l->length == 0 || l->head == NULL;
}

void list_destroy(list** l, void(*delete_value)(void* value)){
    while((*l)->head != NULL){
        node* current = (*l)->head;
        free(current->key);
        if(delete_value != NULL)
            delete_value(current->value);
        (*l)->head = current->next;
        free(current);
    }
    free((*l));
}

void list_insert(list** l, char* key, void* value){
    char* duplicate_key = str_create(key);

    if((*l)->head == NULL){
        insert_head(&(*l)->head, duplicate_key, value);
        (*l)->tail = (*l)->head;
    }else{
        insert_tail(&(*l)->tail, duplicate_key, value);
    }
    (*l)->length++;
}

node* list_getNode(list* l, char* key){
    node* head = l->head;
    node* tail = l->tail;

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

int list_getLength(list* l){
    return l->length;
}

bool list_containsKey(list* l, char* key){
    return list_getNode(l, key) != NULL;
}