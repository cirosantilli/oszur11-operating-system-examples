/*! linker script for memory layout of kernel */

/* OUTPUT_FORMAT("elf32-i386") */

ENTRY(arch_start)

SECTIONS {
	.kernel 0x100000 :
	{
		kernel_code = .;

		*?/arch/i386/arch/startup.asm.o ( .text* )
		*( EXCLUDE_FILE ( *?/arch/syscall.asm.o *?/programs/?* )
			.text* )

		kernel_data = .;

		*( EXCLUDE_FILE ( *?/arch/syscall.asm.o *?/programs/?* )
			.rodata* .data* )
	}
	.kernel_bss :
	{
		*( EXCLUDE_FILE ( *?/arch/syscall.asm.o *?/programs/?* )
			.bss* COMMON* )

		. = ALIGN (4096);
	}

	k_kernel_end = .;

	/DISCARD/ : { *(.comment) } /* gcc info is discarded */
}
