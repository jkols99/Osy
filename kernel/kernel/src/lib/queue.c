// source: https://www.geeksforgeeks.org/queue-linked-list-implementation/, adopted neccessary changes

// A C program to demonstrate linked list based implementation of queue
#include "lib/queue.h"
#include "lib/print.h"
#include "mm/heap.h"
#include "proc/thread.h"

// A utility function to create a new linked list node.
struct qnode_t* new_node(struct thread_t* k) {
    struct qnode_t* temp = (struct qnode_t*)kmalloc(sizeof(struct qnode_t));
    temp->key = k;
    temp->next = NULL;
    return temp;
}

// A utility function to create an empty queue
struct queue_t* create_queue() {
    struct queue_t* q = (struct queue_t*)kmalloc(sizeof(struct queue_t));
    q->front = q->rear = NULL;
    q->size = 0;
    return q;
}

// The function to add a key k to q
void enqueue(struct queue_t* q, struct thread_t* k) {
    if (q->size == THREAD_STACK_SIZE)
        return;
    // Create a new LL node
    struct qnode_t* temp = new_node(k);

    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }

    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
    q->size++;
}

// Function to remove a thread from given queue q
void dequeue(struct queue_t* q, struct thread_t* thread) {
    // If queue is empty, return NULL.
    if (q->front == NULL)
        return NULL;

    struct qnode_t* temp = q->front;
    struct qnode_t* before_the_temp;

    //find given thread
    while (temp != NULL) {
        if (temp->key == thread)
            break;
        before_the_temp = temp;
        temp = temp->next;
    }

    //if thread is not found, print debug message and return
    if (temp == NULL) {
        printk("Dequeue: Thread not found\n%pT", thread);
        printk("Queue: %pL", q);
        return;
    }

    //else remove given thread and free its memory
    before_the_temp->next = temp->next;
    kfree(temp);
    q->size--;
}