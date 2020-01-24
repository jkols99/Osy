// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <drivers/timer.h>
#include <exc.h>
#include <lib/print.h>
#include <mm/tlb.h>
#include <proc/process.h>
#include <proc/thread.h>

void handle_exception_general(context_t* context) {
    int exception_code_value = cp0_cause_get_exc_code(context->cause);
    if (exception_code_value > 0 && exception_code_value < 4) {
        handle_tlb_refill(context);
        return;
    }

    if (cp0_cause_is_interrupt_pending(context->status, 7)) {
        timer_interrupt_after(10000000);
        thread_yield();
    }

    process_kill();
}

bool interrupts_disable(void) {
    unative_t status = cp0_read(REG_CP0_STATUS);
    cp0_write(REG_CP0_STATUS, status & ~CP0_STATUS_IE_BIT);
    return (status & CP0_STATUS_IE_BIT) > 0;
}

void interrupts_restore(bool enable) {
    unative_t status = cp0_read(REG_CP0_STATUS);
    if (enable) {
        status = status | CP0_STATUS_IE_BIT;
    } else {
        status = status & ~CP0_STATUS_IE_BIT;
    }
    cp0_write(REG_CP0_STATUS, status);
}
