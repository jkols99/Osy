// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _KTEST_H
#define _KTEST_H

#include <debug.h>
#include <lib/print.h>

/*
 * Prefiexes for test output, used later by check_output.py script.
 */
#define KTEST_EXPECTED "[EXPECTED]: "
#define KTEST_BLOCK_EXPECTED "[EXPECTED BLOCK]: "
#define KTEST_ACTUAL "[ ACTUAL ]: "

/** Print header of a kernel test. */
#define ktest_start(name) \
    puts("== KERNEL TEST " name " ==")

/** Print message about passed kernel test. */
#define ktest_passed() \
    puts("\n\nTest passed.\n\n")

/** Print message about passed failed test. */
#define ktest_failed() \
    puts("\n\nTest failed.\n\n")

/** Kernel test assertion.
 *
 * Unlike normal assertion, this one is always checked and machine is
 * terminated when expr does not evaluate to true.
 */
#define ktest_assert(msg, expr) \
    do { \
        if (!(expr)) { \
            puts("Kernel test assertion `" msg "' failed at " __FILE__ ":" QUOTE_ME(__LINE__) ": " #expr); \
            ktest_failed(); \
            machine_halt(); \
        } \
    } while (0)

/** Kernel test assertion for bound checking. */
#define ktest_assert_in_range(msg, value, lower, upper) \
    ktest_assert(msg, ((value) >= (lower)) && ((value) <= (upper)))

/** All kernel test share this signature as only one test is compiled at a time. */
void kernel_test(void);

#endif
