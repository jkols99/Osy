// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _MM_FRAME_H
#define _MM_FRAME_H

#include <adt/list.h>
#include <errno.h>
#include <types.h>

#define FRAME_SIZE 4096

typedef struct frame {
    size_t start_address;
} frame_t;

typedef struct frame_item {
    frame_t* frame;
    link_t my_list_link;
} frame_item_t;

list_t frame_list;

void frame_init(void);
errno_t frame_alloc(size_t count, uintptr_t* phys);
errno_t frame_free(size_t count, uintptr_t phys);
void dump_frame_list(void);

#endif
