// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

/*
 * Basic thread test. Creates one thread that calls yield() several times
 * and then terminates.
 */

#include <ktest.h>
#include <proc/thread.h>
#include <lib/print.h>
#include <proc/scheduler.h>
#include <proc/thread.h>


#define LOOPS 5

static void* empty_function(void* ignored) {
    printk("In the empty function\n");
    scheduler_schedule_next();
    return NULL;
}

static void* empty_worker(void* ignored) {
    for (int i = 0; i < LOOPS; i++) {
        if (i == 2 )
            thread_yield();
        printk("Empty worker loop: %d\n", i);
    }
    ktest_passed();
    return NULL;
}

void kernel_test(void) {
    ktest_start("thread/continue");

    thread_t *worker, *empty;
    errno_t rc = thread_create(&worker, empty_worker, NULL, 0, "test-worker");
    ktest_assert_errno(rc, "thread_create");

    rc = thread_create(&empty, empty_function, NULL, 0, "empty");
    ktest_assert_errno(rc, "thread_create");

    scheduler_schedule_next();
    scheduler_schedule_next();
}
