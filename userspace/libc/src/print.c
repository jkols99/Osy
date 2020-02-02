// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <np/syscall.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

static void print_string(char* str) {
    while (*str != '\0') {
        putchar(*str);
        str++;
    }
}

/** Print single character to console.
 *
 * @param c Character to print.
 * @return Character written as an unsigned char cast to an int.
 */
int putchar(int c) {
    __SYSCALL1(SYSCALL_PRINT_CHAR, (unative_t)c);
    return c;
}

static void print_int(unsigned int n, char options[], int base, bool auto_add_zeros) {
    int counter = 0;
    char buffer[50];
    char* ptr;
    ptr = &buffer[49];
    *ptr = '\0';

    do {
        *--ptr = options[n % base];
        n /= base;
        counter++;
    } while (n != 0);

    if (base == 16 && counter < 8 && auto_add_zeros) {
        for (size_t i = counter; i < 8; i++) {
            *--ptr = '0';
        }
    }

    print_string(ptr);
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
    return ++i;
}

/** Prints given formatted string to console.
 *
 * @param format printf-style formatting string.
 * @return Number of printed characters.
 */
int printf(const char* format, ...) {
    int i = 0;
    const char* it = format;
    va_list argp;
    va_start(argp, format);
    while (*it != '\0') {
        if (*it == '%') {
            it++;
            if (*it == '%') {
                putchar('%');
            }
            if (*it == 's') {
                char* string_to_print = va_arg(argp, char*);
                print_string(string_to_print);
            }
            if (*it == 'u') {
                unsigned int usigned = va_arg(argp, unsigned int);
                char targets[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
                print_int(usigned, targets, 10, false);
            }
            if (*it == 'x') {
                const unsigned int uint = va_arg(argp, const unsigned int);
                char targets[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
                print_int(uint, targets, 16, false);
            }
            if (*it == 'X') {
                const unsigned int uint = va_arg(argp, const unsigned int);
                char targets[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
                print_int(uint, targets, 16, false);
            }
            if (*it == 'd') {
                int integer = va_arg(argp, int);
                if (integer < 0) {
                    putchar('-');

                    integer = -integer;
                }
                char targets[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
                print_int(integer, targets, 10, false);
            }
            if (*it == 'c') {
                const char char_to_print = va_arg(argp, const int);
                putchar(char_to_print);
            }
            //printing hex but autofill zeros at the start
            if (*it == 'a') {
                const unsigned int uint = va_arg(argp, const unsigned int);
                char targets[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
                print_int(uint, targets, 16, true);
            }
        } else
            putchar(*it);
        it++;
        i++;
    }
    va_end(argp);
    return i;
}
