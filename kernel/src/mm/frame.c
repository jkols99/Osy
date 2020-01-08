// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <adt/list.h>
#include <debug/mm.h>
#include <exc.h>
#include <lib/print.h>
#include <mm/frame.h>
#include <mm/heap.h>
#include <types.h>

void print_frame_array(void) {
    printk("Printing array...\n");
    for (size_t i = 0; i < frame_container.last_index; i++) {
        printk("%u-th frame with phys: %p\n", i, frame_container.arr[i].phys);
    }
    printk("Done printing array...\n");
}

/**
 * Initializes frame allocator.
 *
 * Called once at system boot.
 */
void frame_init(void) {
    bool ipl = interrupts_disable();
    frame_container.arr[0] = (frame_t){ START_ADD };
    frame_container.last_index = 1;
    print_frame_array();
    interrupts_restore(ipl);
}

static size_t find_place_for_new_alloc(size_t size) {
    for (size_t i = 1; i < frame_container.last_index - 1; i++) {
        size_t gap = frame_container.arr[i + 1].phys - (frame_container.arr[i].phys + FRAME_SIZE);
        if (gap >= size)
            return i;
    }

    return frame_container.last_index - 1;
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
    if (frame_container.last_index + count >= ARR_LEN) {
        printk("Array full\n");
        return ENOMEM;
    }

    size_t required_mem = FRAME_SIZE * count;
    if (required_mem > mem_left) {
        printk("ENOMEM in frame_alloc\n");
        return ENOMEM;
    }
    size_t starting_index = find_place_for_new_alloc(count * FRAME_SIZE);

    for (size_t i = frame_container.last_index - 1; i > starting_index; i--) {
        frame_container.arr[i + count] = frame_container.arr[i];
    }
    for (size_t i = 0; i < count; i++) {
        frame_container.arr[starting_index + 1 + i] = (frame_t){ frame_container.arr[starting_index].phys + FRAME_SIZE * (i + 1) };
    }
    mem_left -= count * FRAME_SIZE;
    frame_container.last_index += count;

    *phys = frame_container.arr[starting_index].phys + FRAME_SIZE;
    printk("Allocated %u frames at %u\n", count, *phys);
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
    bool ipl = interrupts_disable();
    size_t starting_index = 0;
    for (size_t i = 1; i < frame_container.last_index; i++) {
        if (phys == frame_container.arr[i].phys) {
            starting_index = i;
            break;
        }
    }
    printk("Starting index is %u, last index: %u, count: %u\n", starting_index, frame_container.last_index, count);

    if (starting_index == 0)
        return ENOENT;
    if (starting_index + count > frame_container.last_index)
        return EBUSY;

    for (size_t i = starting_index; i < starting_index + count; i++) {
        frame_container.arr[i] = frame_container.arr[i + count];
    }

    mem_left += count * FRAME_SIZE;
    frame_container.last_index -= count;

    interrupts_restore(ipl);
    return EOK;
}
