// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <np/syscall.h>
#include <stdio.h>
#include <../../../kernel/include/drivers/printer.h>

/** Print single character to console.
 *
 * @param c Character to print.
 * @return Character written as an unsigned char cast to an int.
 */
int putchar(int c) {
    printer_putchar('a'); // TODO malo by tu byt namiesto 'a', c* ale mi to nefunguje
    return c;
}

/** Prints given string to console, terminating it with newline.
 *
 * @param s String to print.
 * @return Number of printed characters.
 */
int puts(const char* s) {
    int i = 0;
    while (*s != '\0') {
        putchar(*s);
        s++;
        i++;
    }
    putchar('\n');
    return i;
}

/** Prints given formatted string to console.
 *
 * @param format printf-style formatting string.
 * @return Number of printed characters.
 */
int printf(const char* format, ...) {
    int i = 0;
    while (*format != '\0') {
        putchar(*format);
        format++;
        i++;
    }
    return i;
}
