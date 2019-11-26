// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include "lib/queue.h"
#include <debug.h>
#include <proc/scheduler.h>
#include <lib/print.h>

/** Initialize support for scheduling.
 *
 * Called once at system boot.
 */
void scheduler_init(void) {
    queue = create_queue();
}

/** Marks given thread as ready to be executed.
 *
 * It is expected that this thread would be added at the end
 * of the queue to run in round-robin fashion.
 *
 * @param thread Thread to make runnable.
 */
void scheduler_add_ready_thread(thread_t* thread) {
    enqueue(queue, thread);
}

/** Removes given thread from scheduling.
 *
 * Expected to be called when thread is suspended, terminates execution,
 * etc.
 *
 * @param thread Thread to remove from the queue.
 */
bool scheduler_remove_thread(thread_t* thread) {
    return dequeue(queue, thread);
}

/** Switch to next thread in the queue. */
void scheduler_schedule_next(void) {
    // printk("Schedule next queue: \n");
    // dump_queue_info(queue);
    thread_t* next_thread = get_next_ready(queue, 0);
    if (next_thread == NULL)
        next_thread = get_next_ready(queue, 2);
    printk("OUR NEXT READY: %s \n", next_thread->name);
    // printk("Ready to switch\n");
    thread_switch_to(next_thread);
}

void dump_queue_info(queue_t* queue)
{
    qnode_t* temp = queue->front;
    printk("Dumping queue:\n");
    while (temp != NULL)
    {
        printk("Thread: %s with status %d\n", temp->key->name, temp->key->status);
        temp = temp->next;
    }
    printk("Dumping ends\n");
}
