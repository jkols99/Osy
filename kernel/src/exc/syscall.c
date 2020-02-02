// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <drivers/printer.h>
#include <drivers/timer.h>
#include <exc.h>
#include <lib/print.h>
#include <mm/heap.h>
#include <proc/process.h>
#include <proc/thread.h>

// Upon sucess, shift EPC by 4 to move to the next instruction
// (unlike e.g. TLBL, we do not want to restart it).
void handle_syscall(context_t* context) {
    // __asm__ volatile(".insn\n\n.word 0x29\n\n");
    size_t process_id = context->v0;
    if (process_id == SYSCALL_PRINT_CHAR) {
        printer_putchar((char)context->a0);
        context->epc += 4;
        return;
    }
    process_t* current_process = get_current_process();
    printk("In handle syscall, process_id %u, epc: %p\n", process_id, context->epc);
    if (process_id == SYSCALL_EXIT) {
        printk("syscallexit\n");
        thread_kill(current_process->thread);
    } else if (process_id == SYSCALL_PROC_INFO) {
        np_proc_info_t* info = (np_proc_info_t*)context->a0;
        if (current_process != NULL) {
            current_process->proc_info->total_ticks++;
            *info = *current_process->proc_info;
        } else {
            info = NULL;
        }
    } else if (process_id == SYSCALL_VIRT_MEM) {
        np_proc_info_t* info = current_process->proc_info;
        if (info != NULL) {
            context->v0 = (unative_t)info->virt_mem_size;
        }
    } else if (process_id == SYSCALL_PROC_MEM) {
        if (current_process == NULL) {
            printk("null prpoc\n");
        }
        if (current_process->proc_info == NULL) {
            printk("null proc info\n");
        }
        size_t virt_mem_size = current_process->proc_info->virt_mem_size;
        printk("1\n");
        context->v0 = (unative_t)virt_mem_size;
    }

    context->epc += 4;
}
