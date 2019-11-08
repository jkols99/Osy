// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <main.h>
#include <mm/heap.h>
#include <debug/code.h>
typedef struct Item {
    struct Item* next;
    size_t mem_amount;
} mem_chunk;

mem_chunk* head;

void* kmalloc(size_t size) {
    mem_chunk* temp = head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    mem_chunk* new_mem = NULL;
    new_mem->mem_amount = size;
    temp->next = new_mem;
    void* address = (void*)debug_get_stack_pointer();
    int ret;
    __asm__ volatile("addi $29 $29 %1;" : "=r"(ret) : "r"(size));
    return address;
}

void kfree(void* ptr) {
}

void heap_init(void) {
    head->mem_amount = 0;
}
