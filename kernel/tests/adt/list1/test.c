// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <adt/list.h>
#include <ktest.h>

typedef struct {
    link_t link;
    const char* name;
    const char* color;
} paw_t;

/*
 * For simplicity, we do not allocate any memory and assume that
 * the parameters are literals that are valid for the whole execution.
 */
static void paw_init(paw_t* paw, const char* name, const char* color) {
    link_init(&paw->link);
    paw->name = name;
    paw->color = color;
}

void kernel_test(void) {
    ktest_start("adt/list1");

    paw_t marshall, chase, skye;
    paw_init(&marshall, "Marshall", "red");
    paw_init(&chase, "Chase", "blue");
    paw_init(&skye, "Skye", "pink");

    list_t paws;
    list_init(&paws);

    ktest_assert("list is empty", list_is_empty(&paws));
    ktest_assert("size is 0", list_get_size(&paws) == 0);

    list_append(&paws, &marshall.link);
    ktest_assert("list is not empty", !list_is_empty(&paws));
    ktest_assert("size is 1", list_get_size(&paws) == 1);

    list_append(&paws, &chase.link);
    list_append(&paws, &skye.link);
    ktest_assert("size is 3", list_get_size(&paws) == 3);

    ktest_passed();
}
