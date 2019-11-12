// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University
#include <adt/list.h>
#include <debug.h>
#include <drivers/printer.h>
#include <lib/print.h>
#include <lib/stdarg.h>
#include <types.h>

/** Prints given formatted string to console.
 *
 * @param format printf-style formatting string.
 */
void printk(const char* format, ...) {
    const char* it = format;
    va_list argp;
    va_start(argp, format);
    while (*it != '\0') {
        if (*it == '%') {
            it++;
            if (*it == '%') {
                printer_putchar('%');
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
                    printer_putchar('-');

                    integer = -integer;
                }
                char targets[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
                print_int(integer, targets, 10, false);
            }
            if (*it == 'c') {
                const char char_to_print = va_arg(argp, const int);
                printer_putchar(char_to_print);
            }
            if (*it == 'p') {
                it++;
                char targets[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
                if (*it == 'L') {
                    unsigned int head = va_arg(argp, unsigned int);
                    printer_putchar('0');
                    printer_putchar('x');
                    print_int(head, targets, 16, false);
                    list_t* list = (list_t*)head;
                    link_t* curr = list->head.next;
                    size_t ln = list_get_size(list);
                    if (ln > 0) {
                        printer_putchar('[');
                        print_int(ln, targets, 10, false);
                        printer_putchar(':');
                        printer_putchar(' ');
                        for (size_t i = 0; i < ln; i++) {
                            printer_putchar('0');
                            printer_putchar('x');
                            print_int((int)curr, targets, 16, false);
                            if (i + 1 != ln)
                                printer_putchar('-');
                            curr = curr->next;
                        }
                        printer_putchar(']');
                    } else {
                        print_string("[empty]");
                    }

                } else {
                    const unsigned int pointer_to_print = va_arg(argp, const unsigned int);
                    printer_putchar('0');
                    printer_putchar('x');
                    print_int(pointer_to_print, targets, 16, false);
                    continue;
                }
            }
            //printing hex but autofill zeros at the start
            if (*it == 'a') {
                const unsigned int uint = va_arg(argp, const unsigned int);
                char targets[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
                print_int(uint, targets, 16, true);
            }
        } else
            printer_putchar(*it);
        it++;
    }
    va_end(argp);
}

void print_int(unsigned int n, char options[], int base, bool auto_add_zeros) {
    int counter = 0;
    static char buffer[50];
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

void print_string(char* str) {
    while (*str != '\0') {
        printer_putchar(*str);
        str++;
    }
}

/** Prints given string to console, terminating it with newline.
 *
 * @param s String to print.
 */
void puts(const char* s) {
    while (*s != '\0') {
        printer_putchar(*s);
        s++;
    }
    printer_putchar('\n');
}
