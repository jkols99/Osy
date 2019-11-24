// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _PROC_SCHEDULER_H
#define _PROC_SCHEDULER_H

#include <lib/queue.h>
#include <proc/thread.h>

queue_t* queue;

void scheduler_init(void);
void scheduler_add_ready_thread(thread_t* id);
void scheduler_remove_thread(thread_t* id);
void scheduler_schedule_next(void);

#endif
