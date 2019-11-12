// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <debug/code.h>
#include <debug/mm.h>
#include <lib/print.h>
#include <main.h>
#include <mm/heap.h>
#include <types.h>

void* kmalloc(size_t size) {
    if (init == false)
        heap_init();

    if (heap.last_index == ARR_LENGTH) {
        printk("Heap is full...\n");
        return NULL;
    }

    size_t allign_diff = (4 - (size % 4)) % 4;
    size += allign_diff;
    if (mem_left < size) {
        printk("Returning null pointer, mem_left was %u and size was %u\n", mem_left, size);
        return NULL;
    }
    mem_left -= size;
    size_t real_last_index = heap.last_index - 1;
    printk("Dumping heap array from 0 to %u\n", real_last_index);
    for (size_t i = 0; i < real_last_index; i++) {
        printk("%u-th element with address %u and mem amount %u\n", i, heap.arr[i].address, heap.arr[i].mem_amount);
    }
    printk("Dumping ended\n\n");

    size_t current_adress = heap.arr[real_last_index].address + heap.arr[real_last_index].mem_amount;
    push_back(current_adress, size);

    printk("Returing address: %u\n", current_adress);
    return (void*)current_adress;
}

void kfree(void* ptr) {
    printk("Memory free starts...\n");

    size_t freed_memory = delete_chunk((size_t)ptr);
    printk("Found %u memory to free...\n", freed_memory);

    mem_left += freed_memory;
    printk("Memory free ends...\n\n");
}

void heap_init(void) {
    //get total memory
    mem_left = debug_get_base_memory_size();
    size_t start_address = (size_t)_kernel_end;
    printk("Mem left: %u\n", mem_left);
    printk("Start add: %u\n", start_address);
    heap.arr[0] = (struct mem_chunk){ 0, start_address };
    heap.last_index = 1;
    init = true;
}

void push_back(size_t address, size_t mem) {
    heap.arr[heap.last_index++] = (struct mem_chunk){ mem, address };
}

//returns amount of memory that was freed
size_t delete_chunk(size_t address) {

    size_t add_index = 0;
    //finding index of the item being removed
    for (size_t i = 1; i < heap.last_index; i++) {
        if (address == heap.arr[i].address) {
            add_index = i;
            break;
        }
    }

    //if we havent found target address, return
    if (add_index == 0)
        return 0;

    size_t target_memory = heap.arr[add_index].mem_amount;

    for (size_t i = add_index; i < heap.last_index; i++) {
        heap.arr[i] = heap.arr[i + 1];
    }

    --heap.last_index;

    return target_memory;
}
