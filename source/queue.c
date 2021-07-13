#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include "../lib/queue.h"

pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;

queue* queue_create(){
    queue *q = malloc(sizeof(queue));

    if(q == NULL){
        fprintf(stderr, "Impossible to allocate the queue\n");
        exit(errno);
    }
    q->head = NULL;
    q->tail = NULL;

    return q;
}

void queue_insert_head(node_queue **head, int value){
    node_queue *element = (node_queue *) malloc(sizeof(node_queue));

    if(element == NULL){
        fprintf(stderr, "Impossible to create a new element\n");
        exit(errno);
    }
    element->value = value;
    element->next = *head;

    *head = element;
}

void queue_insert_tail(node_queue **tail, int value){
    node_queue *element = (node_queue *) malloc(sizeof(node_queue));

    if(element == NULL){
        fprintf(stderr, "Impossible to create a new element\n");
        exit(errno);
    }
    element->value = value;
    element->next = NULL;

    (*tail)->next = element;
    *tail = element;
}

pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
static bool queue_closed = false;

void queue_close(queue **q){
    queue_closed = true;
    pthread_cond_broadcast(&queue_cond);
}

int queue_get(queue **q){
    pthread_mutex_lock(&queue_lock);

    while(queue_isEmpty((*q)) && !queue_closed){
        pthread_cond_wait(&queue_cond, &queue_lock);
    }

    if(queue_closed){
        pthread_mutex_unlock(&queue_lock);
        return -1;
    }

    node_queue *curr = (*q)->head;

    if(curr == NULL){
        pthread_mutex_unlock(&queue_lock);
        return -1;
    }

    node_queue *succ = (*q)->head->next;

    if(succ == NULL){
        int r = curr->value;
        free(curr);
        free(*q);
        *q = queue_create();
        pthread_mutex_unlock(&queue_lock);
        return r;
    }

    (*q)->head = curr->next;
    int res = curr->value;
    free(curr);
    pthread_mutex_unlock(&queue_lock);

    return res;
}

bool queue_isEmpty(queue *q){
    return q->head == NULL;
}

void queue_insert(queue **q, int value){
    pthread_mutex_lock(&queue_lock);

    if((*q)->head == NULL){
        queue_insert_head(&(*q)->head, value);
        (*q)->tail = (*q)->head;
    }else{
        queue_insert_tail(&(*q)->tail, value);
    }

    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_lock);
}

void queue_destroy(queue **q) {
    while (!queue_isEmpty((*q))) {
        int c = queue_get(q);
        close(c);
    }

    free(*q);
}
