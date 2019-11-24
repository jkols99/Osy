// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <debug/mm.h>
#include <main.h>

#define CONST_INC 1024

/** Probe available base physical memory.
 *
 * Do not check for non-continuous memory blocks or for memory available via
 * TLB only.
 *
 * @return Amount of memory available in bytes.
 */
size_t debug_get_base_memory_size(void) {
    size_t total = 0;
    
    volatile char* start = (char*)&_kernel_end + 1;

    while ( *start != -1)
    {
        total += CONST_INC;
        start += CONST_INC;
    }

    return total - CONST_INC;
}
