// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _MM_HEAP_H
#define _MM_HEAP_H

#include <types.h>

typedef struct Item {
    struct Item* next;
    size_t mem_amount;
} mem_chunk;

mem_chunk* head;
mem_chunk* new_mem;
size_t mem_left;

void* kmalloc(size_t size);
void kfree(void* ptr);
void heap_init(void);

#endif
