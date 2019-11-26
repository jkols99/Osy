#ifndef _QUEUE_H
#define _QUEUE_H

#include "proc/thread.h"

// A linked list (LL) node to store a queue entry
typedef struct qnode qnode_t;

struct qnode {
    thread_t* key;
    struct qnode* next;
};

// The queue, front stores the front node of LL and rear stores the
// last node of LL
typedef struct queue queue_t;

struct queue {
    qnode_t *front, *rear;
    size_t size;
};

queue_t* create_queue(void);
qnode_t* new_node(thread_t* k);
void enqueue(queue_t* q, thread_t* k);
void dequeue(queue_t* q, thread_t* thread);
#endif