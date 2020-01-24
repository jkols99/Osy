// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <exc.h>
#include <lib/print.h>
#include <mm/as.h>
#include <mm/tlb.h>
#include <proc/scheduler.h>
#include <proc/thread.h>

void handle_tlb_refill(context_t* context) {
    bool ipl = interrupts_disable();

    thread_t* current_thread = get_current_thread();
    as_t* current_as = thread_get_as(current_thread);

    if (current_as == NULL) {
        printk("AS null in refill\n");
        kill_thread(current_thread, false);
        return;
    }

    size_t phys;
    errno_t err = as_get_mapping(current_as, context->badva, &phys);
    if (err != EOK) {
        thread_kill(current_thread);
    }
    size_t virtual_address = context->badva & 0xfffff000;

    size_t virtual_even = 0;
    if (virtual_address % 8192 == 0)
        virtual_even = virtual_address;
    else
        virtual_even = virtual_address - PAGE_SIZE;

    size_t even_mapping;
    size_t odd_mapping;
    errno_t even_err = as_get_mapping(current_as, virtual_even, &even_mapping);
    errno_t odd_err = as_get_mapping(current_as, virtual_even + PAGE_SIZE, &odd_mapping);

    int32_t even_shifted = even_err == EOK ? (int32_t)(even_mapping >> 12) : -1;
    int32_t odd_shifted = odd_err == EOK ? (int32_t)(odd_mapping >> 12) : -1;

    if (even_err == EOK || odd_err == EOK) {
        uint32_t vpn = (virtual_even >> 12);
        uint32_t vpn2 = vpn / 2;

        cp0_write_pagemask_4k();
        cp0_write_entrylo0(even_shifted, true, even_shifted != -1, false);
        cp0_write_entrylo1(odd_shifted, true, odd_shifted != -1, false);
        cp0_write_entryhi(vpn2, current_as->asid);
        cp0_tlb_write_random();
    }

    interrupts_restore(ipl);
}
