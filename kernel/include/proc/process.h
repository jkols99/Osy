// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _PROC_PROCESS_H
#define _PROC_PROCESS_H

#include <errno.h>
#include <proc/thread.h>
#include <types.h>

/** Virtual address of the entry point to userspace application. */
#define PROCESS_ENTRY_POINT 0x00004000

/** Virtual address where the application binary is mounted in MSIM. */
#define PROCESS_IMAGE_START 0xBFB00000

/** Size of the application binary. */
#define PROCESS_IMAGE_SIZE (1024 * 128)

#ifndef PROCESS_MEMORY_SIZE
/** Amount of virtual memory to give to the userspace process. */
#define PROCESS_MEMORY_SIZE (PROCESS_IMAGE_SIZE * 2)
#endif

#if PROCESS_MEMORY_SIZE < PROCESS_IMAGE_SIZE
#error "Cannot give less memory than image size!"
#endif

typedef struct {
    unative_t id;
    size_t virt_mem_size;
    size_t total_ticks;
} np_proc_info_t;

typedef enum {
    SYSCALL_EXIT,
    SYSCALL_PRINT_CHAR,
    SYSCALL_PROC_INFO,
    SYSCALL_PROC_MEM,
    SYSCALL_LAST
} syscall_t;

typedef struct process process_t;

/** Information about existing process. */
struct process {
    thread_t* thread;
    status_t status;
    np_proc_info_t proc_info;
    process_t* following; // process i am waiting for
    process_t* follower; // process that is waiting for me
};

process_t* current_process;

inline process_t* get_current_process(void) { return current_process; }
errno_t process_create(process_t** process, uintptr_t image_location, size_t image_size, size_t process_memory_size);
errno_t process_join(process_t* process, int* exit_status);
void process_kill(void);

#endif
