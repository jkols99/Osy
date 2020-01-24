// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _MM_AS_H
#define _MM_AS_H

#include <adt/list.h>
#include <errno.h>
#include <types.h>

#define PAGE_SIZE 4096
#define ARR_LN 50

typedef struct as {
    size_t size;
    size_t asid;
    size_t phys;
} as_t;

typedef struct as_container {
    as_t arr[ARR_LN];
    size_t last_index;
} as_container_t;

as_container_t as_container;
size_t next_asid;

void as_init(void);
as_t* as_create(size_t size, unsigned int flags);
size_t as_get_size(as_t* as);
void as_destroy(as_t* as);
errno_t as_get_mapping(as_t* as, uintptr_t virt, uintptr_t* phys);

as_t* find_as(size_t asid);
void print_as_container(void);
#endif
