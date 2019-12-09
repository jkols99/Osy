// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include "lib/queue.h"
#include <debug.h>
#include <drivers/timer.h>
#include <exc.h>
#include <lib/print.h>
#include <proc/mutex.h>
#include <proc/scheduler.h>

/** Initialize support for scheduling.
 *
 * Called once at system boot.
 */
void scheduler_init(void) {
    interrupts_disable();
    queue = create_queue();
    timer_interrupt_after(200000);
    interrupts_restore(false);
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

/** Switch to next thread in the queue.
 * First looks for ready threads
 * Then looks for waiting threads
 */
void scheduler_schedule_next(void) {
    interrupts_disable();
    thread_t* next_thread = get_next_ready(queue, 0);
    if (next_thread == NULL)
        next_thread = get_next_ready(queue, 2);
    interrupts_restore(false);
    thread_switch_to(next_thread);
}
