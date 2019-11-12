// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <debug/code.h>
#include <debug/mm.h>
#include <lib/print.h>
#include <main.h>
#include <mm/heap.h>
#include <types.h>

void* kmalloc(size_t size) {
    //if array is full, return null ( this should not happen, but if )
    if (heap.last_index == ARR_LENGTH) {
        return NULL;
    }

    //alligns memory to be divided by 4
    size_t allign_diff = (4 - (size % 4)) % 4;
    size += allign_diff;
    count_biggest_free_block();
    //if we cant allocate size, we return NULL
    if (biggest_free_block < size) {
        return NULL;
    }
    mem_left -= size;
    //gets new address for this malloc
    void* address = (void*)push_back(size);
    return address;
}

void kfree(void* ptr) {
    size_t freed_memory = delete_chunk((size_t)ptr);
    //adds freed memory back to the total memory
    mem_left += freed_memory;
}

//updates biggest continouous block of memory
void count_biggest_free_block(void) {
    size_t gap_sum = 0;
    size_t biggest_gap = 0;
    //sums all gaps in order to count memory after last index + finds biggest gap between memory chunks
    for (size_t i = 0; i < heap.last_index - 1; i++) {
        size_t gap = heap.arr[i + 1].address - (heap.arr[i].address + heap.arr[i].mem_amount);
        gap_sum += gap;
        if (gap > biggest_gap) {
            biggest_gap = gap;
        }
    }
    size_t ending_mem = mem_left - gap_sum;
    //updates the biggest continuous block according to bigger value found
    if (ending_mem > biggest_gap) {
        biggest_free_block = ending_mem;
    } else {
        biggest_free_block = biggest_gap;
    }
}

//initializes important global variables required to run this allocator
void heap_init(void) {
    mem_left = debug_get_base_memory_size();
    size_t start_address = (size_t)_kernel_end;
    heap.arr[0] = (struct mem_chunk){ 0, start_address };
    heap.last_index = 1;
    biggest_free_block = mem_left;
}

size_t push_back(size_t mem) {
    size_t best_index = heap.last_index;
    size_t smallest_remaining_space = UINT_MAX;
    //finds best fit for given memory
    for (size_t i = 1; i < heap.last_index - 1; i++) {
        size_t current_remaining_space = heap.arr[i + 1].address - (heap.arr[i].address + heap.arr[i].mem_amount + mem);
        if (current_remaining_space < smallest_remaining_space) {
            smallest_remaining_space = current_remaining_space;
            best_index = i;
        }
    }

    //if not found, append at the end
    if (best_index == heap.last_index) {
        size_t current_address = heap.arr[best_index - 1].address + heap.arr[best_index - 1].mem_amount;
        heap.arr[best_index] = (struct mem_chunk){ mem, current_address };
        heap.last_index++;
        return current_address;
    } else {
        //else push array left from best index and insert new memory chunk in
        for (size_t j = heap.last_index; j > best_index + 1; j--) {
            heap.arr[j] = heap.arr[j - 1];
        }
        size_t current_address = heap.arr[best_index].address + heap.arr[best_index].mem_amount;
        heap.last_index++;
        heap.arr[best_index + 1] = (struct mem_chunk){ mem, current_address };
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

    // pushes whole array left and omits target memory chunk
    for (size_t i = add_index; i < heap.last_index; i++) {
        heap.arr[i] = heap.arr[i + 1];
    }

    --heap.last_index;

    return target_memory;
}

//prints array for testing purposes
void print_array(void) {
    printk("Header is address: %u, mem: %u\n", heap.arr[0].address, heap.arr[0].mem_amount);
    for (size_t i = 1; i < heap.last_index; ++i) {
        printk("i: %u, address: %u, mem: %u\n", i, heap.arr[i].address, heap.arr[i].mem_amount);
    }
}
