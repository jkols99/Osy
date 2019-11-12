// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include "../theap.h"
#include <ktest.h>
#include <lib/print.h>
#include <mm/heap.h>
#include <types.h>

#define MIN_SIZE 16
#define START_SIZE (MIN_SIZE << 12)
#define MAX_ALLOCATIONS (1024 * 1024 * 1024 / START_SIZE) + (START_SIZE / MIN_SIZE)
#define LOOPS 2

/*
 * Tests that kmalloc() eventually exhausts memory, frees it back to the
 * system and tries once again. Proper allocator must allow that such
 * operation can be tried several times.
 */

static size_t exhaust_and_free(void) {
    uintptr_t previous_block = 0;

    size_t allocation_size = START_SIZE;
    size_t allocation_count = 0;
    while (1) {
        // Stress proper alignment a little bit ;-)
        uintptr_t* ptr = kmalloc(allocation_size - 1);
        if (ptr != NULL) {
            printk("Allocating: %u\n", allocation_size - 1);
            printk("Alligning to: %u\n", allocation_size);
        }
        if (ptr == NULL) {
            allocation_size = allocation_size / 2;
            printk("Lowering allocation size from %u to %u\n", allocation_size * 2, allocation_size);
            if (allocation_size < MIN_SIZE) {
                break;
            }
            continue;
        }

        ktest_check_kmalloc_result(ptr, allocation_size - 1);
        ktest_check_kmalloc_writable(ptr);
        allocation_count++;
        if (allocation_count % 7 == 0)
            printk("Allocation count: %u\n", allocation_count);
        if (allocation_count > MAX_ALLOCATIONS) {
            printk("Too many (%d) allocations succeeded.\n", allocation_count);
            ktest_failed();
            // Unreachable
            return 0;
        }

        ptr[0] = previous_block;
        previous_block = (uintptr_t)ptr;
    }
    printk("Final allocation count: %u\n", allocation_count);

    dprintk("Freeing it back (starting at %x)\n", previous_block);

    // Free it back
    while (previous_block != 0) {
        uintptr_t* ptr = (uintptr_t*)previous_block;
        previous_block = ptr[0];
        kfree(ptr);
    }

    return allocation_count;
}

void kernel_test(void) {
    ktest_start("heap/exhaust2");

    size_t last_count = exhaust_and_free();
    ktest_assert(last_count > 0, "no allocation succeeded");
    printk("Allocated %d.\n", last_count);
    for (int i = 0; i < LOOPS; i++) {
        size_t count = exhaust_and_free();
        printk("Allocated %d.\n", count);
        ktest_assert(count == last_count,
                "allocation counts differ (%u => %u)", last_count, count);
    }

    ktest_passed();
}
