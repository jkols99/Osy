// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University
#include <types.h>

#ifndef _LIB_PRINT_H
#define _LIB_PRINT_H

void printk(const char* format, ...);
void puts(const char* s);
void print_int(unsigned int n, char options[], int base, bool auto_add_zeros);
void print_string(char* str);

#endif
