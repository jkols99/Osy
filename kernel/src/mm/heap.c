// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <debug/code.h>
#include <debug/mm.h>
#include <exc.h>
#include <lib/print.h>
#include <main.h>
#include <mm/heap.h>
#include <types.h>

void* kmalloc(size_t size) {
    //if array is full, return null ( this should not happen, but if )
    bool ipl = interrupts_disable();
    if (heap.last_index == ARR_LENGTH) {
        interrupts_restore(ipl);
        return NULL;
    }

    //alligns memory to be divided by 4
    size_t allign_diff = (4 - (size % 4)) % 4;
    size += allign_diff;
    size_t biggest_free_block = count_biggest_free_block();
    //if we cant allocate size, we return NULL
    if (biggest_free_block < size) {
        interrupts_restore(ipl);
        return NULL;
    }
    mem_left -= size;
    // printk("Mem left was %u and now is %u, size allocated: %u\n", mem_left + size, mem_left, size);
    //gets new addreshis malloc
    size_t address = 0;
    if (biggest_gap_index == 0) {
        address = push_first(size);
    } else {
        address = push_back(size);
    }
    interrupts_restore(ipl);
    return (void*)address;
}

// finds index of first continuous block of given size
size_t find_first_continuous_block(size_t size) {
    for (size_t i = 0; i < heap.last_index - 1; i++) {
        size_t gap = heap.arr[i + 1].address - (heap.arr[i].address + heap.arr[i].mem_amount);
        if (gap >= size)
            return i;
    }
    return heap.last_index - 1;
}

void kfree(void* ptr) {
    bool ipl = interrupts_disable();
    size_t freed_memory = delete_chunk((size_t)ptr);
    //adds freed memory back to the total memory
    mem_left += freed_memory;
    interrupts_restore(ipl);
}

//updates biggest continouous block of memory
size_t count_biggest_free_block(void) {
    bool ipl = interrupts_disable();
    size_t gap_sum = 0;
    size_t biggest_gap = 0;
    biggest_gap_index = UINT_MAX;
    // print_array();
    //sums all gaps in order to count memory after last index + finds biggest gap between memory chunks
    for (size_t i = 1; i < heap.last_index - 1; i++) {
        size_t gap = heap.arr[i + 1].address - (heap.arr[i].address + heap.arr[i].mem_amount);
        if (gap > 0)
            gap_sum += gap;
        if (gap > biggest_gap) {
            biggest_gap = gap;
            biggest_gap_index = i;
        }
    }
    size_t ending_mem = mem_left - gap_sum;
    // printk("Biggest gap index is: %u\n", biggest_gap_index);
    // printk("Total memory is: %u, biggest gap: %u and ending mem: %u\n",mem_left, biggest_gap, ending_mem);
    //updates the biggest continuous block according to bigger value found
    if (ending_mem > biggest_gap) {
        interrupts_restore(ipl);
        return ending_mem;
    } else {
        interrupts_restore(ipl);
        return biggest_gap;
    }
}

//initializes important global variables required to run this allocator 256 1024 4098
void heap_init(void) {
    bool ipl = interrupts_disable();
    mem_left = debug_get_base_memory_size();
    size_t start_address = (size_t)&_kernel_end;
    heap.arr[0] = (struct mem_chunk){ 0, start_address, HEADER };
    heap.last_index = 1;
    interrupts_restore(ipl);
}

size_t push_first(size_t size) {
    bool ipl = interrupts_disable();
    for (size_t i = heap.last_index; i > 1; i--) {
        heap.arr[i] = heap.arr[i - 1];
    }

    size_t current_address = heap.arr[0].address + heap.arr[0].mem_amount;

    heap.arr[1] = (struct mem_chunk){ size, current_address, CLASSIC };

    heap.last_index++;
    interrupts_restore(ipl);
    return current_address;
}

size_t push_back(size_t mem) {
    bool ipl = interrupts_disable();

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
        heap.arr[best_index] = (struct mem_chunk){ mem, current_address, CLASSIC };
        heap.last_index++;
        // printk("IF: Ret address: %p\n", current_address);
        interrupts_restore(ipl);
        return current_address;
    } else {
        // printk("Best index is %u\n", best_index);
        //else push array left from best index and insert new memory chunk in
        for (size_t j = heap.last_index; j > best_index + 1; j--) {
            heap.arr[j] = heap.arr[j - 1];
        }
        size_t current_address = heap.arr[best_index].address + heap.arr[best_index].mem_amount;
        heap.last_index++;
        heap.arr[best_index + 1] = (struct mem_chunk){ mem, current_address, CLASSIC };
        // printk("ELSE: Ret address: %p\n", current_address);
        interrupts_restore(ipl);
        return current_address;
    }
}

//returns amount of memory that was freed
size_t delete_chunk(size_t address) {
    bool ipl = interrupts_disable();
    size_t add_index = 0;
    //finding index of the item being removed
    for (size_t i = 1; i < heap.last_index; i++) {
        if (address == heap.arr[i].address && heap.arr[i].type == CLASSIC) {
            add_index = i;
            break;
        }
    }

    //if we havent found target address, return
    if (add_index == 0) {
        interrupts_restore(ipl);
        return 0;
    }

    size_t target_memory = heap.arr[add_index].mem_amount;

    // pushes whole array left and omits target memory chunk
    for (size_t i = add_index; i < heap.last_index; i++) {
        heap.arr[i] = heap.arr[i + 1];
    }

    --heap.last_index;

    interrupts_restore(ipl);
    return target_memory;
}

//prints array for testing purposes
void print_array(void) {
    bool ipl = interrupts_disable();
    printk("Header is address: %u, mem: %u\n", heap.arr[0].address, heap.arr[0].mem_amount);
    for (size_t i = 1; i < heap.last_index; ++i) {
        printk("i: %u, address: %u, mem: %u\n", i, heap.arr[i].address, heap.arr[i].mem_amount);
    }
    interrupts_restore(ipl);
}