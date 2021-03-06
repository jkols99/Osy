// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

/*
 * Basic thread kill test. Creates one thread and kills it
 * from another.
 */

#include <ktest.h>
#include <proc/thread.h>

#define LOOPS 5

static void* endless_worker(void* ignored) {
    while (true) {
        thread_yield();
    }

    ktest_failed();

    return NULL;
}

void kernel_test(void) {
    ktest_start("thread/kill");

    thread_t* worker;
    errno_t err = thread_create(&worker, endless_worker, NULL, 0, "endless");
    ktest_assert_errno(err, "thread_create");

    err = thread_kill(worker);
    ktest_assert_errno(err, "thread_kill");

    err = thread_join(worker, NULL);
    ktest_assert(err == EKILLED, "thread_join should signal killed thread");

    ktest_passed();
}
