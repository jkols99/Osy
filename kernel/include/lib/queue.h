#ifndef _QUEUE_H
#define _QUEUE_H

#include "proc/thread.h"

// A linked list (LL) node to store a queue entry
struct qnode_t {
    thread_t* key;
    struct qnode_t* next;
};

// The queue, front stores the front node of LL and rear stores the
// last node of LL
struct queue_t {
    struct qnode_t *front, *rear;
    size_t size;
};

struct qnode_t* new_node(thread_t k);
struct queue_t* create_queue();
void enqueue(struct queue_t* q, thread_t* k);
void dequeue(struct queue_t* q, struct thread_t* thread);
#endif