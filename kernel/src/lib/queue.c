// source: https://www.geeksforgeeks.org/queue-linked-list-implementation/, adopted neccessary changes

// A C program to demonstrate linked list based implementation of queue
#include "lib/queue.h"
#include "lib/print.h"
#include "mm/heap.h"
#include "proc/thread.h"

// A utility function to create a new linked list node.
qnode_t* new_node(thread_t* k) {
    qnode_t* temp = (qnode_t*)kmalloc(sizeof(qnode_t));
    temp->key = k;
    temp->next = NULL;
    return temp;
}

// A utility function to create an empty queue, chceme link_t, moze dojst pamat
queue_t* create_queue(void) {
    queue_t* q = (queue_t*)kmalloc(sizeof(queue_t));
    q->front = q->rear = NULL;
    return q;
}

// The function to add a key k to q
void enqueue(queue_t* q, thread_t* k) {
    // Create a new LL node
    qnode_t* temp = new_node(k);

    // If queue is empty, then new node is front and rear both
    if (q->front == NULL) {
        q->front = q->rear = temp;
        return;
    }

    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
}

// Function to remove a thread from given queue q
void dequeue(queue_t* q, thread_t* thread) {
    // If queue is empty, return NULL.
    if (q->front == NULL) // 0 in queue
        return;

    qnode_t* temp = q->front;
    qnode_t* before_the_temp = NULL;

    //find given thread
    while (temp != NULL) {
        if (temp->key == thread)
            break;
        before_the_temp = temp;
        temp = temp->next;
    }

    //if thread is not found, return
    if (temp == NULL) 
        return;

    //else remove given thread and free its memory
    if (before_the_temp != NULL) // 2+ in queue
    {
        before_the_temp->next = temp->next;
        if (temp->next == NULL) // temp is last
            q->rear = before_the_temp;
    }
    else { // deleting first one
        if (temp->next == NULL) // 1 in queue
            q->front = q->rear = NULL;
        else // more in queue
            q->front = q->front->next;       
    }

    kfree(temp);
}

thread_t* get_next_ready(queue_t* where, int status) {
    qnode_t* front = where->front;

    while(front != NULL && front->key->status != status ) {
        front = front-> next;
    }
    
    if (front == NULL)
        return NULL;
    else
        return front->key;
}
