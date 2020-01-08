// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <lib/print.h>
#include <mm/tlb.h>
#include <proc/scheduler.h>
#include <proc/thread.h>

void handle_tlb_refill(context_t* context) {
    printk("In tlb handle\n");
    thread_t* current_thread = get_current_thread();
    thread_kill(current_thread);
    scheduler_schedule_next();
}
