// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <adt/list.h>
#include <debug/mm.h>
#include <drivers/machine.h>
#include <exc.h>
#include <lib/print.h>
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
    frame_container.arr[0] = (frame_t){ START_ADD };
    frame_container.last_index = 1;
    interrupts_restore(ipl);
}

void print_frame_array() {
    for (size_t i = 0; i < frame_container.last_index; i++) {
        printk("Frame number %u, with phys %p\n", i, frame_container.arr[i].phys);
    }
}

static size_t find_place_for_new_alloc(size_t address) {
    for (size_t i = 1; i < frame_container.last_index - 1; i++) {
        if (frame_container.arr[i].phys > address)
            return i - 1;
    }
    return frame_container.last_index - 1;
}

size_t allign_to_4k(size_t value) {
    return value + (FRAME_SIZE - (value % FRAME_SIZE));
}

static size_t find_place_on_heap(size_t count) {
    for (size_t i = 1; i < heap.last_index - 1; i++) {
        size_t ith_offset = heap.arr[i].address + heap.arr[i].mem_amount;
        size_t ref_offset = allign_to_4k(ith_offset);
        if (ref_offset > heap.arr[i + 1].address)
            continue;

        if (heap.arr[i + 1].address - ref_offset >= count * FRAME_SIZE) {
            size_t starting_address = ref_offset;
            kmalloc_at(i, count, starting_address);
            return starting_address - DIFF;
        }
    }
    size_t last_offset = heap.arr[heap.last_index - 1].address + heap.arr[heap.last_index - 1].mem_amount;
    size_t last_start_address = allign_to_4k(last_offset);
    if (last_offset + count * FRAME_SIZE >= first_offset_outside_memory) {
        return 0;
    }
    kmalloc_at(heap.last_index - 1, count, last_start_address);
    return last_start_address - DIFF;
}

// static void check_overlaps() {
//     for (size_t i = 1; i < frame_container.last_index; i++) {
//         for (size_t j = i + 1; j < frame_container.last_index; j++) {
//             if (frame_container.arr[i].phys == frame_container.arr[j].phys) {
//                 printk("Overlap on indices %u and %u\n", i, j);
//                 print_frame_array();
//                 machine_halt();
//             }
//         }
//     }
// }

// static void check_sort_invariant() {
//     for (size_t i = 1; i < frame_container.last_index - 1; i++) {
//         frame_t f1 = frame_container.arr[i];
//         frame_t f2 = frame_container.arr[i + 1];
//         if (f1.phys > f2.phys) {
//             printk("Not sorted on indices %u and %u\n", i, i + 1);
//             print_frame_array();
//             machine_halt();
//         }
//     }
// }

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
        printk("Frame last index %u, count %u\n", frame_container.last_index, count);
        return ENOMEM;
    }

    if (heap.last_index + count >= ARR_LENGTH) {
        printk("Heap last index %u, count %u\n", heap.last_index, count);
        return ENOMEM;
    }

    size_t required_mem = FRAME_SIZE * count;
    if (required_mem > mem_left) {
        printk("Requried mem: %u, mem_left %u\n", required_mem, mem_left);
        return ENOMEM;
    }

    size_t starting_address = find_place_on_heap(count);

    if (starting_address == 0) {
        printk("Allocation would jump out of memory\n");
        return ENOMEM;
    }

    size_t starting_index = find_place_for_new_alloc(starting_address);

    // printk("Frame alloc with count %u at phys %p from index %u\n", count, starting_address, starting_index);
    // printk("Pre-malloc application frame print start\n");
    // print_frame_array();
    // printk("Pre-malloc application frame print ended\n");
    for (size_t i = frame_container.last_index - 1; i > starting_index; i--) {
        frame_container.arr[i + count] = frame_container.arr[i];
    }

    for (size_t i = 0; i < count; i++) {
        frame_container.arr[starting_index + 1 + i] = (frame_t){ starting_address + FRAME_SIZE * i };
    }

    mem_left -= required_mem;
    frame_container.last_index += count;

    *phys = starting_address;
    // check_sort_invariant();
    // check_overlaps();
    // printk("After malloc print start\n");
    // print_frame_array();
    // printk("After malloc print end\n");
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
    size_t starting_index = ARR_LEN + 1;
    for (size_t i = 1; i < frame_container.last_index; i++) {
        if (phys == frame_container.arr[i].phys) {
            starting_index = i;
            break;
        }
    }

    if (starting_index == ARR_LEN + 1) {
        return ENOENT;
    }

    if (starting_index + count > frame_container.last_index) {
        return EBUSY;
    }

    // printk("Freeing %u frames from %p from indices [%u, %u)\n", count, phys, starting_index, starting_index + count);
    // printk("For cycle from %u to %u\n", starting_index, frame_container.last_index - 1 - count);
    for (size_t i = starting_index; i < frame_container.last_index - count; i++) {
        frame_container.arr[i] = frame_container.arr[i + count];
    }

    for (size_t i = 0; i < count; i++) {
        kfree((void*)(phys + i * FRAME_SIZE + DIFF));
    }

    frame_container.last_index -= count;

    interrupts_restore(ipl);
    return EOK;
}
