/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2019 Charles University */

OUTPUT_ARCH(mips)
ENTRY(__entry)

SECTIONS {
	/* Userspace application image assembled from text and data sections */
	
	/*
	 * We start the image at 0x4000 to provide space for NULL page
	 * (that shall not be mapped) and also for stack of the userspace
	 * thread.
	 * While this is not very flexible, it is simple enough to start
	 * with and have userspace application running.
	 */
	.app 0x00000000 : {
	    *(.pre_entry_space)
	}
	.app 0x00004000 : {
		*(.entry)
		
		/* The usual sections */
		
		*(.text .text.*)
		*(.data)
		*(.rodata .rodata.*)
		*(.bss .bss.*)
		*(COMMON)
		_app_end = .;
	}
	
	/* Debugging information copied from default linker script */
	
	.stab            0 : { *(.stab) }
	.stabstr         0 : { *(.stabstr) }
	.stab.excl       0 : { *(.stab.excl) }
	.stab.exclstr    0 : { *(.stab.exclstr) }
	.stab.index      0 : { *(.stab.index) }
	.stab.indexstr   0 : { *(.stab.indexstr) }
	.comment         0 : { *(.comment) }
	.debug           0 : { *(.debug) }
	.line            0 : { *(.line) }
	.debug_srcinfo   0 : { *(.debug_srcinfo) }
	.debug_sfnames   0 : { *(.debug_sfnames) }
	.debug_aranges   0 : { *(.debug_aranges) }
	.debug_pubnames  0 : { *(.debug_pubnames) }
	.debug_info      0 : { *(.debug_info .gnu.linkonce.wi.*) }
	.debug_abbrev    0 : { *(.debug_abbrev) }
	.debug_line      0 : { *(.debug_line) }
	.debug_frame     0 : { *(.debug_frame) }
	.debug_str       0 : { *(.debug_str) }
	.debug_loc       0 : { *(.debug_loc) }
	.debug_macinfo   0 : { *(.debug_macinfo) }
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
	.debug_pubtypes  0 : { *(.debug_pubtypes) }
	.debug_ranges    0 : { *(.debug_ranges) }
	.gnu.attributes  0 : { KEEP (*(.gnu.attributes)) }
	
	/*
	 * Discarded sections
	 *
	 * Some sections need to be explicitly discarded as a
	 * workaround for objcopy errors. These sections are
	 * not needed in the resulting binary or even make no
	 * sense in this context anyway.
	 */
	
	/DISCARD/ : {
		*(.eh_frame)
		*(.reginfo)
		*(.MIPS.abiflags)
	}
}
