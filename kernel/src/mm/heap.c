// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <main.h>
#include <mm/heap.h>
typedef struct Item {
    struct Item* next;
    size_t offset;
    size_t mem_amount;
} mem_chunk;

mem_chunk* head;

void* kmalloc(size_t size) {
    return NULL;
}

void kfree(void* ptr) {
}

void heap_init() {
    head->offset = &_kernel_end;
    head->mem_amount = 0;
}
