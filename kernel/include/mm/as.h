// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _MM_AS_H
#define _MM_AS_H

#include <adt/list.h>
#include <errno.h>
#include <types.h>

#define PAGE_SIZE 4096

typedef struct as {
    size_t size;
    size_t asid;
    uintptr_t va_from;
    uintptr_t phys_from;
    uintptr_t phys_to;
} as_t;

typedef struct as_item {
    as_t* as;
    link_t my_list_link;
} as_item_t;

list_t as_list;
uintptr_t lo0;
uintptr_t lo1;
uintptr_t hi;
size_t next_asid;

void as_init(void);
as_t* as_create(size_t size, unsigned int flags);
size_t as_get_size(as_t* as);
void as_destroy(as_t* as);
errno_t as_get_mapping(as_t* as, uintptr_t virt, uintptr_t* phys);

#endif
