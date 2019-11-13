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

    void* ptr1 = kmalloc(32);
    void* ptr2 = kmalloc(32);
    void* ptr3 = kmalloc(32);

    ktest_assert(ptr1 != NULL, "no memory available");
    ktest_assert(ptr3 != NULL, "no memory available");
    ktest_check_kmalloc_result(ptr1, 16);
    ktest_check_kmalloc_result(ptr3, 16);

    ktest_passed();
}
