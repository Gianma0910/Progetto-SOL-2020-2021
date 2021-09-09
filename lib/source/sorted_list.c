#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include "../sorted_list.h"

sorted_list* sortedlist_create(){
    sorted_list* sl = malloc(sizeof(sorted_list));

    if(sl == NULL){
        fprintf(stderr,"Impossible to create a new sorted list: malloc error\n");
        exit(errno);
    }
    sl->head = NULL;
    sl->tail = NULL;
    sl->size = 0;

    return sl;
}

elem* n = NULL;

static void insert_head(elem** head, int value){
    elem* element = (elem*)malloc(sizeof(elem));

    if(elem == NULL){
        fprintf(stderr, "Impossible to create a new element: malloc error\n");
        return;
    }
    element->value = value;
    element->next = *head;

    *head = element;
}

static void insert_tail(elem** tail, int value){
    elem* element = (elem*)malloc(sizeof(elem));

    if(elem == NULL){
        fprintf(stderr, "Impossible to create a new element: malloc error\n");
        return;
    }
    element->value = value;
    element->next = NULL;

    (*tail)->next = element;
    *tail = element;
}

static void insert_middle(elem** head, int value){
    elem* current = *head;
    elem* next_node = current->next;
    elem* element = (elem*)malloc(sizeof(elem));

    if(element == NULL){
        fprintf(stderr, "Impossible to create a new element: malloc error\n");
        return;
    }
    element->value = value;

    while(!(current->value <= value && value <= next_node->value)){
        next_node = next_node->next;
        current = current->next;
    }
    element->next = next_node;
    current->next = element;
}

bool sortedlist_remove(sorted_list** sl, int value){
    elem** head = &(*sl)->head;

    elem* current = *head;
    elem* next_elem = current->next;

    if(current->value == value){
        *head = current->next;
        free(current);
        (*sl)->size--;
        return true;
    }

    if(next_elem == NULL){
        return false;
    }

    while(next_elem->value != value && current != NULL){
        current = current->next;
        next_elem = next_elem->next;
        if(next_elem == NULL)
            break;
    }

    if(current != NULL && next_elem == NULL){
        if(current->value == value){
            free(current);
            (*sl)->size--;
            return true;
        }
    }else if(current != NULL){
        current->next = next_elem->next;
        if(next_elem->next == NULL)
            (*sl)->tail = current;
        free(current);
        (*sl)->size--;
        return true;
    }

    return false;
}

bool sortedlist_isEmpty(sorted_list* sl){
    return sl->size == 0;
}

void sortedlist_destroy(sorted_list** sl){
    while((*sl)->head != NULL){
        elem* current = (*sl)->head;
        (*sl)->head = current->next;
        free(current);
    }

    free((*sl));
}

void sortedlist_insert(sorted_list** sl, int value){
    if(sortedlist_isEmpty((*sl))){
        insert_head(&(*sl)->head, value);
        (*sl)->tail = (*sl)->head;
    }else{
        if((*sl)->head->value >= value){
            insert_head(&(*sl)->head, value);
        }else if((*sl)->tail->value <= value){
            insert_tail(&(*sl)->tail, value);
        }else{
            insert_middle(&(*sl)->head, value);
        }
    }
    (*sl)->size++;
}

void sortedlist_iterate(){
    n = NULL;
}

int sortedlist_getNext(sorted_list* sl){
    if(n == NULL){
        n = sl->head;
    }

    int value = n->value;
    n = n->next;

    return value;
}

int sortedlist_getMax(sorted_list* sl){
    return sl->tail->value;
}