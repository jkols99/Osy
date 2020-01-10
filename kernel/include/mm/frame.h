// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _MM_FRAME_H
#define _MM_FRAME_H

#include <errno.h>
#include <types.h>

#define FRAME_SIZE 4096
#define START_ADD 0x00000000
#define ARR_LEN 200
#define DIFF 0x80000000

typedef struct frame {
    size_t phys;
} frame_t;

typedef struct frame_container {
    frame_t arr[ARR_LEN];
    size_t last_index;
} frame_container_t;

frame_container_t frame_container;

void frame_init(void);
errno_t frame_alloc(size_t count, uintptr_t* phys);
errno_t frame_free(size_t count, uintptr_t phys);

void print_frame_array(void);
size_t allign_to_4k(size_t value);
#endif
