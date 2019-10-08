// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <debug/mm.h>
#include <main.h>

/** Probe available base physical memory.
 *
 * Do not check for non-continuous memory blocks or for memory available via
 * TLB only.
 *
 * @return Amount of memory available in bytes.
 */
size_t debug_get_base_memory_size(void) {
    return 0;
}
