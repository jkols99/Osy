// source: https://www.geeksforgeeks.org/queue-linked-list-implementation/, adopted neccessary changes

// A C program to demonstrate linked list based implementation of queue
#include "lib/queue.h"
#include "lib/print.h"
#include "mm/heap.h"
#include "proc/thread.h"
#include <exc.h>

// A utility function to create a new linked list node.
qnode_t* new_node(thread_t* k) {
    bool ipl = interrupts_disable();
    qnode_t* temp = (qnode_t*)kmalloc(sizeof(qnode_t));
    temp->key = k;
    temp->next = NULL;
    interrupts_restore(ipl);
    return temp;
}

// A utility function to create an empty queue, chceme link_t, moze dojst pamat
queue_t* create_queue(void) {
    bool ipl = interrupts_disable();
    queue_t* q = (queue_t*)kmalloc(sizeof(queue_t));
    if (q == NULL)
    {
        interrupts_restore(ipl);
        return NULL;
    }
    q->front = q->rear = NULL;
    interrupts_restore(ipl);
    return q;
}

// The function to add a key k to q
void enqueue(queue_t* q, thread_t* k) {
    // Create a new LL node
    bool ipl = interrupts_disable();
    qnode_t* temp = new_node(k);

    // If queue is empty, then new node is front and rear both
    if (q->front == NULL) {
        q->front = q->rear = temp;
        interrupts_restore(ipl);
        return;
    }

    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
    interrupts_restore(ipl);
}

// Function to remove a thread from given queue q
bool dequeue(queue_t* q, thread_t* thread) {
    // If queue is empty, return NULL.
    bool ipl = interrupts_disable();
    if (q->front == NULL) // 0 in queue
    {
        interrupts_restore(ipl);
        return false;
    }

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
    {
        interrupts_restore(ipl);
        return false;
    }

    //else remove given thread and free its memory
    if (before_the_temp != NULL) // 2+ in queue
    {
        before_the_temp->next = temp->next;
        if (temp->next == NULL) // temp is last
            q->rear = before_the_temp;
    } else { // deleting first one
        if (temp->next == NULL) // 1 in queue
            q->front = q->rear = NULL;
        else // more in queue
            q->front = q->front->next;
    }

    kfree(temp);
    interrupts_restore(ipl);
    return true;
}

/**
 * Get next thread in queue
 * @param where Where to search
 * @param status Search based on status of the thread
 * @return Next thread to run
*/
thread_t* get_next_ready(queue_t* where, int status) {
    bool ipl = interrupts_disable();
    qnode_t* front = where->front;

    while (1) {
        if (front == NULL)
        {
            interrupts_restore(ipl);
            return NULL;
        }
        if (front->key->status == status && front->key->following == NULL)
        {
            interrupts_restore(ipl);
            return front->key;
        }
        front = front->next;
    }
}

/**
 * Removes all references
 * @param queue Where to remove
 * @param thread_to_kill What references to remove
*/
void remove_all_dependencies(queue_t* queue, thread_t* thread_to_kill) {
    bool ipl = interrupts_disable();
    qnode_t* temp = queue->front;

    while (temp != NULL) {
        if (temp->key->following == thread_to_kill)
            temp->key->following = NULL;
        temp = temp->next;
    }
    interrupts_restore(ipl);
}

thread_t* remove_and_return_first(queue_t* queue) {
    bool ipl = interrupts_disable();
    qnode_t* return_value = queue->front;
    queue->front = queue->front->next;
    interrupts_restore(ipl);
    return return_value->key;
}

size_t queue_size(queue_t* queue) {
    bool ipl = interrupts_disable();
    qnode_t* temp = queue->front;
    size_t counter = 0;

    while (temp) {
        counter++;
        temp = temp->next;
    }
    interrupts_restore(ipl);
    return counter;
}