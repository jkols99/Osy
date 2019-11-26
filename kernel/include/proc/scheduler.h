// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _PROC_SCHEDULER_H
#define _PROC_SCHEDULER_H

#include <lib/queue.h>
#include <proc/thread.h>

queue_t* queue;
thread_t* current_thread;

// getter
inline thread_t* get_current_thread(void) { return current_thread; };
// setter
inline void set_current_thread(thread_t* new_current_thread) { current_thread = new_current_thread; };

inline void set_current_thread_to_finished(void) { current_thread->status = FINISHED; };

void scheduler_init(void);
void scheduler_add_ready_thread(thread_t* id);
bool scheduler_remove_thread(thread_t* id);
void scheduler_schedule_next(void);

/** puts thread to the end of the queue
 * @param target_thread Thread to be moved
*/
inline void rotate(thread_t* target_thread) {
    if (scheduler_remove_thread(target_thread))
        scheduler_add_ready_thread(target_thread);
};

#endif
