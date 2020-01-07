// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <drivers/cp0.h>
#include <exc.h>
#include <lib/print.h>
#include <mm/as.h>
#include <mm/heap.h>

/** Initializes support for address spaces.
 *
 * Called once at system boot
 */
void as_init(void) {
    bool ipl = interrupts_disable();
    list_init(&as_list);
    lo0 = 0x13;
    lo1 = 0x17;
    hi = (uintptr_t)PAGE_SIZE;
    next_asid = 0x1;
    interrupts_restore(ipl);
}

static void increment_globals() {
    bool ipl = interrupts_disable();
    printk("Before: lo0=%d, lo1=%d, hi=%d, next_asid=%d\n", lo0, lo1, hi, next_asid);
    lo0 = lo1;
    lo1 += 4;
    hi += (uintptr_t)PAGE_SIZE;
    next_asid += 1;
    printk("After: lo0=%d, lo1=%d, hi=%d, next_asid=%d\n", lo0, lo1, hi, next_asid);
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
    as_item_t* new_as_item = (as_item_t*)kmalloc(sizeof(as_item_t));
    if (new_as_item == NULL) {
        printk("As item malloc failed\n");
        interrupts_restore(ipl);
        return NULL;
    }
    new_as_item->as = (as_t*)kmalloc(sizeof(as_t));
    if (new_as_item->as == NULL) {
        printk("As malloc failed\n");
        interrupts_restore(ipl);
        return NULL;
    }

    new_as_item->as->size = size;
    new_as_item->as->phys_to = lo1;
    new_as_item->as->phys_from = lo0;
    new_as_item->as->va_from = hi;

    cp0_write_pagemask_4k();
    cp0_write_entrylo0(lo0, true, true, false);
    cp0_write_entrylo1(lo1, true, true, false);
    cp0_write_entryhi(hi, next_asid);
    cp0_tlb_write_random();
    increment_globals();

    list_append(&as_list, &new_as_item->my_list_link);
    interrupts_restore(ipl);
    return new_as_item->as;
}

/** Get size of given address space (in bytes). */
size_t as_get_size(as_t* as) {
    return as->size;
}

/** Destroy given address space, freeing all the memory. */
void as_destroy(as_t* as) {
    bool ipl = interrupts_disable();
    list_foreach(as_list, as_item_t, my_list_link, item) {
        if (item->as == as) {
            list_remove(&item->my_list_link);
            kfree(item->as);
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
    return ENOIMPL;
}
