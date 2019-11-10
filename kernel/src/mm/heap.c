// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <main.h>
#include <mm/heap.h>
#include <debug/code.h>
#include <debug/mm.h>

mem_chunk* new_mem;

void* kmalloc(size_t size) {
    if (debug_get_base_memory_size() < size) {
        return NULL;
    }

    new_mem->amount = size;
    new_mem->address = debug_get_stack_pointer();
    heap_insert(new_mem);
    return (void*)new_mem->address;
}

void kfree(void* ptr) {

}
mem_chunk* heap[debug_get_base_memory_size()];
size_t last;

void heap_init(void) {
    last = -1;
}

void heap_insert(mem_chunk* memChunk) {
    last += 1;
    heap[last] = memChunk;
    bubble_up(last);
}

void bubble_up(size_t index) {
    while (index > 1) {
        mem_chunk* parent = heap[index / 2];
        if (parent->address <= heap[index]->address) {
            break;
        }
        mem_chunk* temp = parent;
        parent = heap[index];
        heap[index] = temp;
        index = index / 2;
    }
}
