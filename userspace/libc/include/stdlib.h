// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _LIBC_STDLIB_H
#define _LIBC_STDLIB_H

#include <stddef.h>

typedef struct mem_chunk {
    size_t mem_amount;
    size_t address;
} mem_chunk_t;

#define ARR_LENGTH 10
#define UINT_MAX 1000000

typedef struct heap {
    mem_chunk_t arr[ARR_LENGTH];
    size_t last_index;
} heap_t;

heap_t heap;
size_t mem_left;
size_t biggest_gap_index;
size_t first_offset_outside_memory;

void heap_init(void);
void exit(int status);
void* malloc(size_t size);
void free(void* ptr);

#endif
