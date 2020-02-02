// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <errno.h>
#include <exc.h>
#include <lib/print.h>
#include <mm/as.h>
#include <mm/frame.h>
#include <mm/heap.h>
#include <proc/process.h>
#include <proc/scheduler.h>
#include <proc/thread.h>
#include <proc/userspace.h>

size_t id = 0;

static void copy_binary_to_virtual(uintptr_t image_location, uintptr_t image_size, uintptr_t start_va) {
    size_t* image_start = (size_t*)0xBFB01000;
    size_t* va_start = (size_t*)0x1000;
    size_t to_be_copied = image_size - 4096;
    // printk("First value at %p is %p\n", image_start, *image_start);
    // printk("Va_start at %p\n", va_start);
    while (to_be_copied > image_size - (4096 * 4)) {
        va_start++;
        image_start++;
        to_be_copied -= sizeof(size_t);
        // printk("va_start %p, image_start %p, to be copied %u\n", va_start, image_start, to_be_copied);
    }
    // printk("Va start: %p, image_Start: %p, to be copied %u, image_size %u\n", va_start, image_start, to_be_copied, image_size);
    // __asm__ volatile(".insn\n\n.word 0x29\n\n");
    while (to_be_copied > 0) {
        *va_start = *image_start;
        if (*image_start != 0x0) {
            // printk("Copy init from %p to %p, size %u, to be copied %u\n", image_start, va_start, image_size, to_be_copied);
            // printk("Data at %p: %p copied from %p\n", va_start, *va_start, *image_start);
        }
        va_start++;
        image_start++;
        to_be_copied -= sizeof(size_t);
    }
    printk("Successful assign\n");
}

static void* entry(void* data) {
    printk("In us entry\n");
    copy_binary_to_virtual(PROCESS_IMAGE_START, PROCESS_IMAGE_SIZE, 0x0);
    // __asm__ volatile(".insn\n\n.word 0x29\n\n");
    cpu_jump_to_userspace((uintptr_t)(0x3000), (uintptr_t)PROCESS_ENTRY_POINT);
    return NULL;
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
    errno_t err = thread_create_new_as(&thread, entry, NULL, 1, "Thread for process", process_memory_size);
    if (err == ENOMEM) {
        interrupts_restore(ipl);
        return ENOMEM;
    }
    // TODO nakopirovat binarku
    np_proc_info_t* info = (np_proc_info_t*)kmalloc(sizeof(np_proc_info_t));
    if (info == NULL) {
        // kfree(thread->stack);
        // kfree(thread->address_space);
        // kfree(thread);
        interrupts_restore(ipl);
        return ENOMEM;
    }
    info->id = id++;
    info->virt_mem_size = allign_to_4k(process_memory_size);
    info->total_ticks = 0;

    process_t* proc = (process_t*)kmalloc(sizeof(process_t));
    if (proc == NULL) {
        // kfree(thread->stack);
        // kfree(thread->address_space);
        // kfree(thread);
        // kfree(info);
        interrupts_restore(ipl);
        return ENOMEM;
    }
    proc->thread = thread;
    proc->status = READY;
    proc->proc_info = info;
    *process_out = proc;
    proc->thread->data = process_out;
    size_t* img_loc = (size_t*)image_location;
    printk("Start byte of img loc %p\n", *img_loc);
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

    current_process = process;
    errno_t join_errno = thread_join(process->thread, NULL);

    interrupts_restore(ipl);
    return join_errno;
}
