// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <adt/list.h>
#include <debug/mm.h>
#include <exc.h>
#include <lib/print.h>
#include <main.h>
#include <mm/frame.h>
#include <mm/heap.h>
#include <types.h>

/**
 * Initializes frame allocator.
 *
 * Called once at system boot.
 */
void frame_init(void) {}

/**
 * Allocate continuous sequence of physical frames.
 *
 * The allocated frames can be returned by frame_free. Note that multiple
 * frames allocated by this function can be returned separate by frame_free
 * and vice versa.
 *
 * @param count How many frames to allocate.
 * @param phys Where to store physical address of the first frame in sequence.
 * @return Error code.
 * @retval EOK Frames allocated.
 * @retval ENOMEM Not enough memory.
 */
errno_t frame_alloc(size_t count, uintptr_t* phys) {
    bool ipl = interrupts_disable();
    size_t required_mem = FRAME_SIZE * count;
    size_t biggest_free_block = count_biggest_free_block();
    printk("Required mem: %u, biggest free: %u\n", required_mem, biggest_free_block);
    if (required_mem > biggest_free_block)
        return ENOMEM;
    size_t starting_offset_index = find_first_continuous_block(FRAME_SIZE * count);
    size_t starting_offset_address = heap.arr[starting_offset_index].address + heap.arr[starting_offset_index].mem_amount;
    for (size_t i = heap.last_index + count - 1; i > starting_offset_index + 1; i--) {
        heap.arr[i] = heap.arr[i - count];
    }

    for (size_t i = 0; i < count; i++) {
        heap.arr[starting_offset_index + 1 + i] = (struct mem_chunk){ FRAME_SIZE, starting_offset_address + FRAME_SIZE * i, FRAME };
        heap.last_index++;
    }
    phys = (uintptr_t*)starting_offset_address;
    mem_left -= required_mem;
    printk("Phys is: %p\n", phys);
    interrupts_restore(ipl);
    return EOK;
}

static errno_t remove_frames_from_index(size_t target_index, size_t count) {
    for (size_t i = 0; i < count; i++) {
        mem_chunk current_mem_chunk = heap.arr[target_index + i];
        if (current_mem_chunk.type != FRAME)
            return EBUSY;
        current_mem_chunk = heap.arr[target_index + count];
        heap.last_index--;
        mem_left -= FRAME_SIZE;
    }
    return EOK;
}

/**
 * Free continuous sequence of physical frames.
 *
 * The returned frames were previously allocated by frame_alloc. Note that
 * multiple frames allocated by separate calls to frame_alloc can be freed
 * at once by this function and vice versa.
 *
 * @param count How many frames to free.
 * @param phys Physical address of the first frame in sequence.
 * @return Error code.
 * @retval EOK Frames freed.
 * @retval ENOENT Invalid frame address.
 * @retval EBUSY Some frames were not allocated (double free).
 */
errno_t frame_free(size_t count, uintptr_t phys) {
    // printk("Freeing %u frames from %p starting address\n", count, phys);
    bool ipl = interrupts_disable();
    errno_t ret_val = EOK;
    for (size_t i = 1; i < heap.last_index - 1; i++) {
        if (phys == heap.arr[i].address) {
            ret_val = remove_frames_from_index(i, count);
            break;
        }
    }
    interrupts_restore(ipl);
    return ret_val;
}