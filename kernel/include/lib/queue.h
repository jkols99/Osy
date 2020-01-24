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
};

queue_t* create_queue(void);
qnode_t* new_node(thread_t* k);
thread_t* get_next_ready(queue_t* where, int status);
void enqueue(queue_t* q, thread_t* k);
bool dequeue(queue_t* q, thread_t* thread);
void remove_all_dependencies(queue_t* queue, thread_t* thread_to_kill);
thread_t* remove_and_return_first(queue_t* queue);
size_t queue_size(queue_t* queue);
#endif