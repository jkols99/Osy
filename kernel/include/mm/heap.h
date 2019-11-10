// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _MM_HEAP_H
#define _MM_HEAP_H

#include <types.h>

void* kmalloc(size_t size);
void kfree(void* ptr);
typedef struct Item {
    size_t address;
    size_t amount;
} mem_chunk;

void heap_init(void);
void heap_insert(mem_chunk* memChunk);
void heap_delete(size_t index);
int heap_find(mem_chunk memChunk);
void bubble_up(size_t last);

#endif
