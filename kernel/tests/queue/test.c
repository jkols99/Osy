// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <ktest.h>
#include <lib/print.h>
#include <lib/queue.h>

/*
 * Tests that kmalloc() returns a valid address. We assume
 * there would be always enough free memory to allocate 8 bytes
 * when the kernel starts.
 */

void kernel_test(void) {
    ktest_start("queue/basic");

    queue_t* queue = create_queue();

    thread_t* t1 = (thread_t*)kmalloc(sizeof(thread_t));
    t1->name[0] = "Thread1";
    t1->entry_func = NULL;
    enqueue(queue, t1);
    printk("%pL", queue);

    // ktest_assert((size_t)ptr4 == sizeT, "Wrong fit");
    // ktest_assert(ptr1 != NULL, "no memory available");
    // ktest_assert(ptr3 != NULL, "no memory available");
    // ktest_check_kmalloc_result(ptr1, 16);
    // ktest_check_kmalloc_result(ptr3, 16);

    ktest_passed();
}
