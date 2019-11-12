// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include "../theap.h"
#include <ktest.h>
#include <mm/heap.h>

/*
 * Tests that kmalloc() returns a valid address. We assume
 * there would be always enough free memory to allocate 8 bytes
 * when the kernel starts.
 */

void kernel_test(void) {
    ktest_start("heap/basic");

    void* ptr1 = kmalloc(16);
    void* ptr2 = kmalloc(16);
    size_t sizeT= (size_t)ptr2;
    void* ptr3 = kmalloc(16);
    kfree(ptr2);
    void* ptr4 = kmalloc(8);
    void* ptr5 = kmalloc(8);
    printk("Ptr1: %u\n",(size_t)ptr1);
    printk("Ptr4: %u\n",(size_t)ptr4);
    printk("Ptr5: %u\n",(size_t)ptr5);
    printk("Ptr3: %u\n",(size_t)ptr3);
    printk("sizeT + 8: %u\n", sizeT + 8);
    printk("sizeT: %u\n", sizeT);
    ktest_assert((size_t)ptr4 == sizeT, "Wrong fit");
    ktest_assert(ptr1 != NULL, "no memory available");
    ktest_assert(ptr3 != NULL, "no memory available");
    ktest_check_kmalloc_result(ptr1, 16);
    ktest_check_kmalloc_result(ptr3, 16);

    ktest_passed();
}
