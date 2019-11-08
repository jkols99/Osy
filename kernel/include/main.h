// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _MAIN_H
#define _MAIN_H

#include <types.h>

void kernel_main(void);

/* See definition in kernel linker script. */
extern uint8_t* _kernel_end;

#endif
