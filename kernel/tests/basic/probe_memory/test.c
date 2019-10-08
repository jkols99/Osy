// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <debug.h>
#include <debug/mm.h>
#include <ktest.h>
#include <main.h>

/*
 * Test that available memory is detected correctly.
 * Note that this relies on passing correct size via macro
 * KERNEL_TEST_PROBE_MEMORY_MAINMEM_SIZE_KB.
 */

/* Detection accuracy within 4KB is okay. */
#define THRESHOLD_KB 4

void kernel_test(void) {
    ktest_start("basic/probe_memory");

#ifndef KERNEL_TEST_PROBE_MEMORY_MAINMEM_SIZE_KB
#error Macro KERNEL_TEST_PROBE_MEMORY_MAINMEM_SIZE_KB not defined
#endif

    size_t detected_kb = debug_get_base_memory_size() / 1024;
    size_t actual_size_kb = KERNEL_TEST_PROBE_MEMORY_MAINMEM_SIZE_KB;
    dprintk("mainmem is %dKB, detected %dKB\n", actual_size_kb, detected_kb);

    actual_size_kb = actual_size_kb - ((((uintptr_t)&_kernel_end) - 0x80000000) / 1024);
    ktest_assert_in_range("memory size detection okay", detected_kb,
            actual_size_kb - THRESHOLD_KB, actual_size_kb + THRESHOLD_KB);

    ktest_passed();
}
