# Assignment 01: Introduction to the Kernel

**This is the first assignment that requires the MSIM emulator and the MIPS
cross compiler toolchain. Please see
[the course website](https://d3s.mff.cuni.cz/teaching/nswi004) for the download
and installation instructions.**

The goal of the first assignment is to code your first functions in your future
kernel running on the MIPS R4000 processor, executing in a simulated environment
using the MSIM emulator. The assignment builds on the previous one and extends
it, you should certainly reuse most of your code.

Read the assignment carefully. There are multiple tasks to implement.

This is an individual assignment. Do not change the provided API and do not
modify the logic of the tests. It is, however, perfectly fine to use the tests
for debugging or to add your own tests.



## Assignment


### Provided tests

Your implementation will be primarily checked via automated tests. For the
kernel, these reside in the `tests` subdirectory, each test is implemented as
a function `void kernel_test(void)` in one `test.c` file and called from the
`void kernel_main(void)` function of your kernel.

For simplicity, the tests are executed using a script:

```shell
./tools/tester.py suite --verbose suite_as1.txt
```

The script compiles and links the kernel anew for each test.

You can also run single test via:

```shell
./tools/tester.py kernel --verbose adt/list1
```


### Provided lists

`kernel/adt/list.h` contains a simple list implementation, also available in
the Linux kernel and other low-level C projects. Unlike traditional containers
used in C++ or Java, the list implementation embeds the link structure `link_t`
as a member in the list item type, and not the other way around. This decreases
memory fragmentation, simplifies dynamic allocation, but also requires separate
`link_t` members for each list an item can be a member of.

Consult the tests to see how one can work with such list.


### Formatted printing

In kernel, the `printf` analogue is usually called `printk`. Your first task is
to port your implementation of `printf` from the previous task to your kernel.

As a standard feature, add `%p` to print a pointer as an address in hexadecimal
format prefixed with `0x` (see the tests for details).

As a non-standard addition, add a formatting directive for printing lists.
Using the convention from the Linux kernel, when `%p` is followed by `L`,
consider the argument to be of type `list_t *` and print addresses of all
`link_t *` items of this list. See the tests for exact format specification.
Note that when `%p` is followed by a character other than `L`, it should be
copied verbatim.

For future extensions, consider that you may want to add more specifiers for
your own structures into `printk` later.


### Reading the value of the stack pointer

Implement a `debug_get_stack_pointer()` function that reads the current value
of the stack pointer. Examine the `basic/stack_pointer` test to refresh your
knowledge of the stack and the `$sp` register. Try to implement the function
first without using the inline assembly, afterwards investigate the use of
the `register` keyword.


### Determining the size of the available memory

The MSIM simulator consults the `msim.conf` configuration file to determine how
much memory the simulated machine has. This file, however, is not visible to
your kernel, the kernel therefore needs some other way to determine how much
memory there is. Think about what can be done and then implement
a `debug_get_base_memory_size()` function that will return the amount of
available memory.

In your implementation, assume that all memory is available in a single
continuous block that starts with the kernel. The `_kernel_end` symbol,
defined by the linker, gives the address at the end of the kernel image.


### Dumping function binary code

As the very last task, implement a `debug_dump_function()` function that will
dump the binary code of a given function. The dump should should simply show
a given number of the hexadecimal opcodes taken from the start of the function
(that is, you are not expected to convert the opcodes to instruction mnemonics,
and you do not need to determine the actual function size).


## Compilation

Kernel compilation uses the three traditional phases: configuration,
compilation and testing.

The configuration is done by a script called `configure.py`. The script detects
the location of the cross-compilation toolchain (remember, we are compiling
code for the MIPS R4000 processor, hence we cannot use the standard compiler).
The script accepts command line options:

- Adding `--debug` will produce a debug build where the debugging messages are
  printed to the console and assertions are tested. It is highly recommended
  to develop with this setting.

- To compile and execute a kernel test, use the `--kernel-test` option followed
  by the slug of the test name, for example `--kernel-test adt/list1`.

After configuration, the kernel is built via `make`. If `make` finishes without
errors, you can launch your kernel simply by typing `msim`. Incremental `make`
works until the configuration changes. To rebuild the kernel from scratch,
execute `make distclean` and then reconfigure.

Note that `configure.py` can prepare an out-of-tree build.


## Other notes

While we do not enforce any particular coding style, we do require that the code
is consistently formatted. The repository contains a `.clang-format` file that
that contains one possible configuration. Feel free to update the style to
your likings, but note that future assignments will be published as merges
from the upstream repository, extensive formatting changes can therefore
cause merge conflicts.

This repository should have active CI so your tests are automatically executed
after each push.
