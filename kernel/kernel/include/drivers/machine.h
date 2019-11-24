// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _DRIVERS_MSIM_H
#define _DRIVERS_MSIM_H

/** Halts the (virtual) machine. */
static inline void machine_halt(void) {
    __asm__ volatile(".word 0x28\n");
}

#endif
