// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

/*
 * Kernel bootstrap code.
 *
 * This code will be loaded at (physical) address 0.
 */

/*
 * MSIM-specific macro for halting the simulator.
 */
.macro halt
    .insn
    .word 0x28

    /*
     * Loop forever in case the above instruction is not
     * properly recognized by the simulator.
     * As a matter of fact, if the instruction is not recognized,
     * exception should have been raised but we are a bit paranoid
     * here ;-).
     */

    0:
        j 0b
        nop

.endm halt

/*
 * We know what whare are doing here so do not let
 * assembler change our code in any way.
 */

.set noat
.set noreorder

/*
 * Put the hardwired code to a special input section ".excvec".
 * This section is then placed by the linker (as instructed by the kernel
 * linker script) at the very beginning of the kernel image (address 0).
 */

.section .excvec, "ax"

/*
 * Kernel entry point.
 * The bootstrap loader (0x1FC00000) jumps here.
 * We only jump to the C code from here.
 */

.org   0x400
.globl start
.ent   start

start:
	la $sp, 0x80000400
	la $gp, 0x80000000
	jal kernel_main
	nop
	
	/*
	 * Again, paranoid style: this is unreachable anyway. */
	halt
.end start

/*
 * Rest of code.
 */
.text

