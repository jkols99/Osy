# Assignment 02: Heap for the Kernel

## Basic Functions

Check out the `mm/heap.h` header and implement the heap management functions for the kernel.
The `heap_init` function should initialize your heap for later use (this is where your code
for detecting available memory will come in handy). The `kmalloc` and `kfree` functions
should allocate and free a memory block just as standard `malloc` and `free` would do.

## Bump Pointer Heap

Imagine a program that only calls `malloc` but never `free`. Such a program either completes
with whatever memory was available, or runs out of memory. It also makes heap management
implementation very simple - it can simply remember the last free memory location,
`malloc` will move that variable forward as necessary, `free` will not do anything.
This is sometimes called Bump Pointer Allocator, because all it does is it bumps a pointer forward.

To pass the assignment, you can initially implement this type of allocator.
Later on, working `free` will also be needed in addition to working `malloc`,
so expect to return to the allocator to improve it, but right now it is enough
to pass (obviously, certain tests will fail).

## Memory Allocator Tests

The tests are intentionally simple to accept trivial allocator implementation.
You should consider adding add your own tests. The heap management implementation
is quite bug prone and it will be difficult to debug other parts of your kernel if
you cannot be certain that your heap works.

## Assignment Grading

A fully working assignment, that is, with both `kmalloc` and `kfree` doing what they should,
will receive a baseline of 10 points. A working assignment with bump pointer heap, that is,
working `kmalloc` but empty `kfree`, will receive a baseline of 6 points. This refers
to the state of the assignment at the submission deadline.

The baseline will be further adjusted based on additional criteria:

- Bonus for relevant features that were not part of the assignment
  (such as more complex tests, support for debugging allocation errors, and so on).

- Penalty for bugs that were not discovered by the tests but are still important
  (in extreme cases where the code passess the tests more or less by chance
  the penalty can be severe).

- Penalty for technically poor solution even if it works somehow.

- Penalty for poor coding style (such as inconsistent or cryptic variable names,
  lack of comments, poor code structure, and so on).

Any single bonus or penalty will typically be restricted to 1-2 points,
however, exceptions in extreme cases are possible.
