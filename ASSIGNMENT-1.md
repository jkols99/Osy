# Assignment 01: Introduction to the Kernel

As a team, you already have two implementations of the functions required for the first
assignment, which was done individually (one implementation from each team member).

Pick the solution that you like more (or even better, select the best from both)
and copy it to this repository. Refer to the original assignment for what
features need to be ported. A quick recap follows:

**Note**: as a new feature, implement `%u` to print unsigned
integers (this can be used for `size_t` or `uintptr_t` too).

## Required functionality

- formatted printing a.k.a. `printk` with
    - `%d`, `%x`, `%c`, `%s`
    - `%p` and `%pL`
    - `%u`
- reading the value of the stack pointer
- determining size of the available memory
- dumping function binary code (opcode dump)

## Assignment Grading

A fully working assignment will receive a baseline of 10 points.

The baseline will be further adjusted based on additional criteria:

- Bonus for relevant features that were not part of the assignment
  (such as more complex tests, innovative implementation, support
  for more print formatting features, and so on).

- Penalty for bugs that were not discovered by the tests but are still important
  (in extreme cases where the code passess the tests more or less by chance
  the penalty can be severe).

- Penalty for technically poor solution even if it works somehow.

- Penalty for poor coding style (such as inconsistent or cryptic variable names,
  lack of comments, poor code structure, and so on).

Any single bonus or penalty will typically be restricted to 1-2 points,
however, exceptions in extreme cases are possible.
