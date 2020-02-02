// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <drivers/cp0.h>
#include <exc.h>
#include <lib/print.h>
#include <mm/as.h>
#include <mm/frame.h>
#include <mm/heap.h>

/** Initializes support for address spaces.
 *
 * Called once at system boot
 */
void as_init(void) {
    bool ipl = interrupts_disable();
    as_container.last_index = 0;
    next_asid = 1;
    interrupts_restore(ipl);
}

/** Create new address space.
 *
 * @param size Address space size, must be aligned to PAGE_SIZE.
 * @param flags Flags (unused).
 * @return New address space.
 * @retval NULL Out of memory.
 */
as_t* as_create(size_t size, unsigned int flags) {
    bool ipl = interrupts_disable();

    if (as_container.last_index == ARR_LN) {
        printk("As container full\n");
        interrupts_restore(ipl);
        return NULL;
    }

    size_t alligned_size = allign_to_4k(size);

    uintptr_t phys;
    errno_t err = frame_alloc(alligned_size / FRAME_SIZE, &phys);
    if (err == ENOMEM) {
        print_frame_array();
        printk("ENOMEM for new as\n");
        interrupts_restore(ipl);
        return NULL;
    }
    as_t* new_as = (as_t*)kmalloc(sizeof(as_t));
    new_as->size = alligned_size;
    new_as->asid = next_asid++;
    new_as->phys = phys;
    as_container.arr[as_container.last_index++] = *new_as;

    interrupts_restore(ipl);
    return new_as;
}

/** Get size of given address space (in bytes). */
size_t as_get_size(as_t* as) {
    return as->size;
}

/** Destroy given address space, freeing all the memory. */
void as_destroy(as_t* as) {
    bool ipl = interrupts_disable();
    for (size_t i = 1; i < as_container.last_index; i++) {
        if (as == &as_container.arr[i]) {
            uintptr_t phys = as_container.arr[i].phys;
            for (size_t j = i; i < as_container.last_index - 1; j++) {
                as_container.arr[j] = as_container.arr[j + 1];
            }
            as_container.last_index--;
            frame_free(as_container.arr[i].size / PAGE_SIZE, phys);
            return;
        }
    }
    interrupts_restore(ipl);
}

/** Get mapping between virtual pages and physical frames.
 *
 * @param as Address space to use.
 * @param virt Virtual address, aligned to page size.
 * @param phys Where to store physical frame address the page is mapped to.
 * @return Error code.
 * @retval EOK Mapping found.
 * @retval ENOENT Mapping does not exist.
 */
errno_t as_get_mapping(as_t* as, uintptr_t virt, uintptr_t* phys) {
    printk("Virt: %p, as size: %x, virt actual size: %x\n", virt, as->size, 262144);
    if (virt < 0x1000) {
        return ENOENT;
    }

    if (virt >= as->size) {
        return ENOENT;
    }

    *phys = as->phys + virt;
    return EOK;
}

as_t* find_as(size_t asid) {
    for (size_t i = 1; i < as_container.last_index; i++) {
        if (asid == as_container.arr[i].asid)
            return &as_container.arr[i];
    }
    return NULL;
}