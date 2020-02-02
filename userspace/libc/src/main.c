// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <assert.h>
#include <stdlib.h>

extern int main(void);
extern void __main(void);

/** First C code running in userspace. */
void __main() {
    heap_init();
    int ret_val = main();
    exit(ret_val);

    assert(0 && "unreachable code");
}
