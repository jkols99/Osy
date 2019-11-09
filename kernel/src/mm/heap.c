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
mem_chunk* new_mem;

void* kmalloc(size_t size) {
    mem_chunk* temp = head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    new_mem->mem_amount = size;
    new_mem->next = NULL;
    temp->next = new_mem;
    void* address = (void*)debug_get_stack_pointer();
    int ret;
    __asm__ volatile("add $sp, %1, $sp;" : "=r"(ret) : "r"(size));
    return address;
}

void kfree(void* ptr) {
    int offset = (int)_kernel_end;
    mem_chunk* temp = head;
    mem_chunk* before = head;
    int address = (int)ptr;
    while (offset < address) {
        offset += temp->mem_amount;
        before = temp;
        temp = temp->next;
    }
    if (temp == 0) return;
    // remove element from linked list
    before->next = temp->next;
    int amount_of_memory_to_free = (int)temp->mem_amount;
    int ret;
    __asm__ volatile("sub $sp, %1, $sp;" : "=r"(ret) : "r"(amount_of_memory_to_free));
}

void heap_init(void) {
    head->mem_amount = 0;
    head->next = NULL;
}
