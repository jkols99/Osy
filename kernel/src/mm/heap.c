// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <debug/code.h>
#include <debug/mm.h>
#include <lib/print.h>
#include <main.h>
#include <mm/heap.h>
#include <types.h>

void* kmalloc(size_t size) {
    printk("Malloc\n");
    if (init == false)
        heap_init();

    if (heap.last_index == ARR_LENGTH) {
        printk("The end--------------------------------------------------------------------------------------------------\n");
        return NULL;
    }

    size_t allign_diff = (4 - (size % 4)) % 4;
    size += allign_diff;
    countBiggestFreeBlock();
    if (biggest_free_block < size) {
        return NULL;
    }
    mem_left -= size;
    printk("Som pred push_back\n");
    void* address = (void*)push_back(size);
//    print_array();
    printk("Som pred return, addressa je: %u\n", (size_t)address);
    return address;
}

void kfree(void* ptr) {
    printk("Free\n");
    size_t freed_memory = delete_chunk((size_t)ptr);

    mem_left += freed_memory;
//    print_array();
}

void countBiggestFreeBlock(void) {
    size_t gap_sum = 0;
    size_t biggest_gap = 0;
    for (size_t i = 0; i < heap.last_index - 1; i++) {
        size_t gap = heap.arr[i + 1].address - (heap.arr[i].address + heap.arr[i].mem_amount);
        gap_sum += gap;
        if (gap > biggest_gap) {
            biggest_gap = gap;
        }
    }
    size_t ending_mem = mem_left - gap_sum;
    if (ending_mem > biggest_gap) {
        biggest_free_block = ending_mem;
    } else {
        biggest_free_block = biggest_gap;
    }

}

void heap_init(void) {
    //get total memory
    mem_left = debug_get_base_memory_size();
    size_t start_address = (size_t)_kernel_end;
    heap.arr[0] = (struct mem_chunk){ 0, start_address };
    heap.last_index = 1;
    init = true;
    biggest_free_block = mem_left;
}

size_t push_back(size_t mem) {
    size_t best_index = heap.last_index;
    size_t smallest_remaining_space = UINT_MAX;
    for (size_t i = 1; i < heap.last_index - 1; i++) {
        size_t current_remaining_space = heap.arr[i + 1].address - (heap.arr[i].address + heap.arr[i].mem_amount + mem);
        if (current_remaining_space < smallest_remaining_space) {
            smallest_remaining_space = current_remaining_space;
            best_index = i;
        }
    }

    if (best_index == heap.last_index) {
        size_t current_address = heap.arr[best_index - 1].address + heap.arr[best_index - 1].mem_amount;
        heap.arr[best_index] = (struct mem_chunk){ mem, current_address };
        heap.last_index++;
//        printk("IF: Current address: %u\n", current_address);
        return current_address;
    } else {
        for (size_t j = heap.last_index; j > best_index + 1; j--) {
            heap.arr[j] = heap.arr[j - 1];
        }
        size_t current_address = heap.arr[best_index].address + heap.arr[best_index].mem_amount;
        heap.last_index++;
        heap.arr[best_index + 1] = (struct mem_chunk) {mem, current_address};
//        printk("ELSE: Current address: %u\n", current_address);
        return current_address;
    }
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

void print_array(void) {
    printk("Header is address: %u, mem: %u\n", heap.arr[0].address, heap.arr[0].mem_amount);
    for (size_t i = 1; i < heap.last_index; ++i) {
        printk("i: %u, address: %u, mem: %u\n", i, heap.arr[i].address, heap.arr[i].mem_amount);
    }
}
