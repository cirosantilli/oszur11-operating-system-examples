/*! linker script for memory layout of kernel */

/* OUTPUT_FORMAT("elf32-i386") */

ENTRY ( arch_start ) /* entry point, after grub */

SECTIONS {
	.single 0x100000 :
	{
		/* magic number grub looks for, and starup code */
		*?/arch/i386/arch/startup.asm.o ( .text* )

		/* instructions */
		*( .text* )

		/* read only data (constants), initialized global variables */
		* ( .rodata* .data* )

		/* uninitialized global variables (or initialized with 0) */
		* ( .bss* COMMON* )

		. = ALIGN (4096);
	}

	/DISCARD/ : { *(.comment) } /* gcc info is discarded */
}
