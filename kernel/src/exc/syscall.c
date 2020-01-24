// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <drivers/timer.h>
#include <exc.h>
#include <lib/print.h>
#include <proc/process.h>

// Upon sucess, shift EPC by 4 to move to the next instruction
// (unlike e.g. TLBL, we do not want to restart it).
void handle_syscall(context_t* context) {
    size_t process_id = context->v0;
    size_t argument = context->a0;
    process_t* current_process = get_current_process();

    if (process_id == SYSCALL_PROC_INFO) {
        np_proc_info_t* info = (np_proc_info_t*)argument;
        if (current_process != NULL) {
            current_process->proc_info.total_ticks++;
            *info = current_process->proc_info;
        } else {
            info = NULL;
        }
    } else if (process_id == SYSCALL_PROC_MEM) {
        size_t* arg1_ptr = &argument;
        if (current_process != NULL) {
            *arg1_ptr = current_process->proc_info.virt_mem_size;
        } else {
            arg1_ptr = 0;
        }
    }

    context->epc += 4;
}
