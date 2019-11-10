// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <debug/code.h>
#include <debug/mm.h>
#include <main.h>
#include <mm/heap.h>

void* kmalloc(size_t size) {
    if (mem_left < size)
        return NULL;

    mem_chunk* temp = head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    new_mem->mem_amount = size;
    new_mem->next = NULL;
    temp->next = new_mem;
    void* address = (void*)debug_get_stack_pointer();
    mem_left -= size;
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
    if (temp == 0)
        return;
    // remove element from linked list
    before->next = temp->next;
    mem_left += (int)temp->mem_amount;
}

void heap_init(void) {
    mem_left = debug_get_base_memory_size();
    head->mem_amount = 0;
    head->next = NULL;
}
