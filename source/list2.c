#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "../lib/my_string.h"
#include "../lib/list2.h"

list1 *list_create(){
    list2 *l = malloc(sizeof(list));
    if(l == NULL){
        fprintf(stderr, "Impossible to allocate the list\n");
        exit(errno);
    }
    l->head = NULL;
    l->tail = NULL;
    l->length = 0;

    return l;
}

void insert_head(node **head, char *pid_client){
    node *element = (node *) malloc(sizeof(node));
    if(element == NULL){
        fprintf(stderr, "Impossible create a new node\n");
        exit(errno);
    }
    element->pid_client = pid_client;
    element->next = *head;

    *head = element;
}

void insert_tail(node **tail, char *pid_client){
    node *element = (node *) malloc(sizeof(node));

    if(element == NULL){
        fprintf(stderr, "Impossible create a new node\n");
        exit(errno);
    }
    element->pid_client = pid_client;
    element->next = NULL;

    (*tail)->next = element;
    *tail = element;
}

bool list_remove(list2 **l, char *pid_client){
    node **head = &(*l)->head;

    node *curr = *head;
    node *succ = (*head)->next;

    if(str_equals(curr->pid_client, pid_client)){
        *head = curr->next;
        free(curr->pid_client);

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

    while(!str_equals(succ->pid_client, pid_client) && curr != NULL){
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

        free(succ->pid_client);
        free(succ);
        (*l)->length--;

        return true;
    }

    return false;
}

bool list_isEmpty(list2 *l){
    return l->length == 0;
}

void list_destroy(list2 **l){
    while((*l)->head != NULL){
        node *curr = (*l)->head;
        free(curr->pid_client);

        (*l)->head = curr->next;
        free(curr);
    }

    free((*l));
}

void list_insert(list2 **l, char *pid_client){
    char *dup_pid = str_create(pid_client);

    if((*l)->head == NULL){
        insert_head(&(*l)->head, dup_pid);
        (*l)->tail = (*l)->head;
    }else{
        insert_tail(&(*l)->tail, dup_pid);
    }

    (*l)->length++;
}

node* list_getNode(list2 *l, char *pid_client){
    node *head = l->head;
    node *tail = l->tail;

    if(head == NULL){
        return NULL;
    }

    if(str_equals(head->pid_client, pid_client)){
        return head;
    }else if(str_equals(tail->pid_client, pid_client)){
        return tail;
    }

    while(head != NULL && !str_equals(head->pid_client, pid_client)){
        head = head->next;
    }

    if(head != NULL){
        return head;
    }

    return NULL;
}

int list_getLength(list2 *l){
    return l->length;
}

bool list_containsKey(list2 *l, char *pid_client){
    return list_getNode(l, pid_client) != NULL;
}