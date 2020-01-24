// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <errno.h>
#include <exc.h>
#include <lib/print.h>
#include <mm/frame.h>
#include <proc/process.h>
#include <proc/scheduler.h>
#include <proc/thread.h>
#include <proc/userspace.h>

size_t id = 0;

void process_kill() {
    current_process->status = KILLED;
    printk("1\n");
    thread_kill(current_process->thread);
    printk("2\n");
}

/** Create new userspace process.
 *
 * @param process_out Where to place the initialized process_t structure.
 * @param image_location Virtual address (in kernel segment) where is the image of the raw application binary.
 * @param image_size Size of the application binary image.
 * @param process_memory_size Amount of virtual memory to give to the application (at least image_size).
 * @return Error code.
 * @retval EOK Process was created and its main thread started.
 * @retval ENOMEM Not enough memory to complete the operation.
 * @retval EINVAL Invalid call (unaligned size etc.).
 */
errno_t process_create(process_t** process_out, uintptr_t image_location, size_t image_size, size_t process_memory_size) {
    bool ipl = interrupts_disable();
    if (image_location == 0 || image_size == 0 || process_memory_size < image_size) {
        interrupts_restore(ipl);
        return EINVAL;
    }

    thread_t* thread;
    errno_t err = thread_create_new_as(&thread, (void* (*)(void*))image_location, NULL, 1, "Thread for process", process_memory_size);
    qnode_t* temp = queue->front;
    while (1) {
        if (temp == NULL)
            break;
        printk("Thread name: %s, status %u\n", temp->key->name, temp->key->status);
        temp = temp->next;
    }
    if (err == ENOMEM) {
        interrupts_restore(ipl);
        return ENOMEM;
    }
    np_proc_info_t info = { id++, allign_to_4k(process_memory_size), 0 };
    process_t process = { thread, READY, info, NULL, NULL };
    *process_out = &process;

    interrupts_restore(ipl);
    return EOK;
}

/** Wait for termination of another process.
 *
 * @param process Process to wait for.
 * @param exit_status Where to place the process exit status (return value from main).
 * @return Error code.
 * @retval EOK Joined successfully.
 * @retval EBUSY Some other thread is already joining this process.
 * @retval EKILLED Process was killed.
 * @retval EINVAL Invalid process.
 */
errno_t process_join(process_t* process, int* exit_status) {
    bool ipl = interrupts_disable();
    if (process == NULL) {
        interrupts_restore(ipl);
        return EINVAL;
    }

    if (process->status < 0 || process->status > 5) {
        interrupts_restore(ipl);
        return EOK;
    }

    if (process->status == KILLED) {
        interrupts_restore(ipl);
        return EKILLED;
    }

    if (process->follower != NULL) {
        interrupts_restore(ipl);
        return EBUSY;
    }

    if (current_process != NULL) {
        current_process->following = process;
        process->follower = current_process;
    }
    current_process = process;
    set_current_thread(process->thread);
    printk("Jumping to userspace\n");
    cpu_jump_to_userspace((uintptr_t)process->thread->stack_top, (uintptr_t)process->thread->entry_func);

    interrupts_restore(ipl);
    return EOK;
}
