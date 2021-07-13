#ifndef QUEUE_H
#define QUEUE_H
#include <stdbool.h>
typedef struct node_queue{
    int value;
    struct node_queue* next;
}node_queue;

typedef struct queue{
    node_queue* head;
    node_queue* tail;
}queue;

queue *queue_create();
int queue_get(queue **q);
bool queue_isEmpty(queue* q);
void queue_insert(queue **q, int value);
void queue_destroy(queue **q);
void queue_close(queue **q);
#endif