// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _PROC_THREAD_H
#define _PROC_THREAD_H

#include <errno.h>
#include <types.h>
#include <proc/context.h>

/** Thread stack size.
 *
 * Set quite liberally as stack overflows are notoriously difficult to debug
 * (and difficult to detect too).
 */
#define THREAD_STACK_SIZE 4096

/** Max length (excluding terminating zero) of thread name. */
#define THREAD_NAME_MAX_LENGTH 31

/** Thread entry function as you know from pthreads. */
typedef void* (*thread_entry_func_t)(void*);

//defining enum for thread stauts
#define READY 0
#define RUNNING 1
#define WAITING 2
#define SUSPENDED 3
#define FINISHED 4
typedef int status_t;

/** Information about any existing thread. */
typedef struct thread thread_t;
struct thread {
    char name[THREAD_NAME_MAX_LENGTH + 1];
    thread_entry_func_t entry_func;
    status_t status;
    context_t* context; /* maybe */
    void* data;
    void* stack;
    void* stack_top;
    thread_t* following;
    thread_t* follower;
};

void threads_init(void);
errno_t thread_create(thread_t** thread, thread_entry_func_t entry, void* data, unsigned int flags, const char* name);
thread_t* thread_get_current(void);
void thread_yield(void);
void thread_suspend(void);
void thread_finish(void* retval) __attribute__((noreturn));
bool thread_has_finished(thread_t* thread);
errno_t thread_wakeup(thread_t* thread);
errno_t thread_join(thread_t* thread, void** retval);
void thread_switch_to(thread_t* thread);

#endif
