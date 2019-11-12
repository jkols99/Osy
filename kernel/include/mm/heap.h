// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _MM_HEAP_H
#define _MM_HEAP_H

#include <types.h>

typedef struct mem_chunk {
    size_t mem_amount;
    size_t address;
} mem_chunk;

#define ARR_LENGTH 10000

typedef struct my_heap {
    mem_chunk arr[ARR_LENGTH];
    size_t last_index;
} my_heap;

bool init;
struct my_heap heap;
size_t mem_left;

void* kmalloc(size_t size);
void kfree(void* ptr);
void heap_init(void);

//linked list functions
void push_back(size_t address, size_t mem);
size_t delete_chunk(size_t address);

#endif
