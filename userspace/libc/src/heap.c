// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <np/proc.h>
#include <np/syscall.h>
#include <stdio.h>
#include <stdlib.h>

static void print_heap(void) {
    printf("Print start...\n");
    for (size_t i = 0; i < heap.last_index; i++) {
        printf("%u-th with add %x and mem %u\n", i, heap.arr[i].address, heap.arr[i].mem_amount);
    }
    printf("Printing done...\n");
}

static size_t count_biggest_free_block(void) {
    size_t gap_sum = 0;
    size_t biggest_gap = 0;
    biggest_gap_index = ARR_LENGTH + 1;
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
        return ending_mem;
    } else {
        return biggest_gap;
    }
}

void heap_init(void) {
    heap.arr[0] = (mem_chunk_t){ (size_t)_app_end, 0 };
    heap.last_index = 1;
    mem_left = (size_t)__SYSCALL1(SYSCALL_PROC_MEM, (unative_t)_app_end) - (size_t)_app_end;
    size_t info = (size_t)__SYSCALL1(SYSCALL_VIRT_MEM, 0);
    printf("x\n");
    printf("virt mem size %u\n", info);
    printf("xaaa\n");
    first_offset_outside_memory = info;
    printf("y\n");
    printf("mem_left: %u firstoff %u\n", mem_left, first_offset_outside_memory);
    printf("Userland heap [%u,%u)\n", _app_end, first_offset_outside_memory);
}

static size_t push_first(size_t size) {
    for (size_t i = heap.last_index; i > 1; i--) {
        heap.arr[i] = heap.arr[i - 1];
    }

    size_t current_address = heap.arr[0].address + heap.arr[0].mem_amount;

    heap.arr[1] = (mem_chunk_t){ size, current_address };

    heap.last_index++;
    return current_address;
}

static size_t push_back(size_t mem) {

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
        size_t next_new_add = heap.arr[heap.last_index - 1].address + mem;
        printf("New add %x %u, first off %x %u\n", next_new_add, next_new_add, first_offset_outside_memory, first_offset_outside_memory);
        if (next_new_add >= first_offset_outside_memory)
            return 0;

        size_t current_address = heap.arr[best_index - 1].address + heap.arr[best_index - 1].mem_amount;
        heap.arr[best_index] = (mem_chunk_t){ mem, current_address };
        heap.last_index++;
        // printk("IF: Ret address: %p\n", current_address);
        return current_address;
    } else {
        // printk("Best index is %u\n", best_index);
        //else push array left from best index and insert new memory chunk in
        for (size_t j = heap.last_index; j > best_index + 1; j--) {
            heap.arr[j] = heap.arr[j - 1];
        }
        size_t current_address = heap.arr[best_index].address + heap.arr[best_index].mem_amount;
        heap.last_index++;
        heap.arr[best_index + 1] = (mem_chunk_t){ mem, current_address };
        // printk("ELSE: Ret address: %p\n", current_address);
        return current_address;
    }
}

static size_t delete_chunk(size_t address) {
    size_t add_index = 0;
    //finding index of the item being removed
    for (size_t i = 1; i < heap.last_index; i++) {
        if (address == heap.arr[i].address) {
            add_index = i;
            break;
        }
    }

    //if we havent found target address, return
    if (add_index == 0) {
        return 0;
    }

    size_t target_memory = heap.arr[add_index].mem_amount;

    // pushes whole array left and omits target memory chunk
    for (size_t i = add_index; i < heap.last_index; i++) {
        heap.arr[i] = heap.arr[i + 1];
    }

    --heap.last_index;

    return target_memory;
}

void* malloc(size_t size) {
    if (heap.last_index == ARR_LENGTH) {
        printf("Heap full\n");
        return NULL;
    }

    size_t allign_diff = (4 - (size % 4)) % 4;
    size += allign_diff;

    size_t biggest_free_block = count_biggest_free_block();

    if (biggest_free_block < size) {
        return NULL;
    }

    printf("Alligned size %u, biggest_free_block %u, biggest_gap_index %u\n", size, biggest_free_block, biggest_gap_index);
    mem_left -= size;

    size_t address = 0;
    if (biggest_gap_index == 0) {
        address = push_first(size);
    } else {
        address = push_back(size);
    }
    print_heap();
    return (void*)address;
}

void free(void* ptr) {
    size_t freed_memory = delete_chunk((size_t)ptr);
    mem_left += freed_memory;
}
