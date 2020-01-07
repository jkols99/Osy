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
void frame_init(void) {
    bool ipl = interrupts_disable();
    list_init(&frame_list);
    interrupts_restore(ipl);
}

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
    if (required_mem > count_biggest_free_block())
        return ENOMEM;
    size_t starting_offset_index = find_first_continuous_block(FRAME_SIZE * count);
    size_t starting_offset_address = heap.arr[starting_offset_index].address + heap.arr[starting_offset_index].mem_amount;
    for (size_t i = heap.last_index + count - 1; i > starting_offset_index + 1; i--) {
        heap.arr[i] = heap.arr[i - count];
    }

    for (size_t i = 0; i < count; i++) {
        heap.arr[starting_offset_index + 1 + i] = (struct mem_chunk){ FRAME_SIZE, starting_offset_address + FRAME_SIZE * i, FRAME };
    }
    phys = (uintptr_t*)starting_offset_address;
    mem_left -= required_mem;
    printk("Phys is: %p\n", phys);
    // size_t starting_offset = find_optimal_continuous_block(count);
    // phys = (uintptr_t*)starting_offset;
    // for (size_t i = 0; i < count; i++) {
    //     frame_item_t* new_frame_item = (frame_item_t*)kmalloc(sizeof(frame_item_t));
    //     if (new_frame_item == NULL) {
    //         interrupts_restore(ipl);
    //         return ENOMEM;
    //     }

    //     new_frame_item->frame = (frame_t*)kmalloc(sizeof(frame_t));
    //     if (new_frame_item->frame == NULL) {
    //         interrupts_restore(ipl);
    //         return ENOMEM;
    //     }

    //     new_frame_item->frame->start_address = (size_t)starting_offset + FRAME_SIZE * i;
    //     if (i == 0)
    //         phys = &(new_frame_item->frame->start_address);
    //     list_append(&frame_list, &new_frame_item->my_list_link);

    //     mem_left -= FRAME_SIZE;
    // }
    interrupts_restore(ipl);
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
    printk("Freeing %u frames from %p starting address\n", count, phys);
    bool free_blocks = false;
    list_foreach(frame_list, frame_item_t, my_list_link, item) {
        if (item->frame->start_address == phys)
            free_blocks = true;
        if (free_blocks) {
            if (count-- > 0) {
                list_remove(&item->my_list_link);
                kfree(item->frame);
                kfree(item);
                mem_left += FRAME_SIZE;
            } else
                free_blocks = false;
        }
    }
    if (count > 0)
        return EBUSY;

    return EOK;
}

void dump_frame_list(void) {
    size_t x = 0;
    list_foreach(frame_list, frame_item_t, my_list_link, item) {
        printk("Start address of %u-th frame: %p\n", x++, item->frame->start_address);
    }
}