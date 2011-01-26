/*! linker script for memory layout of kernel */

/* OUTPUT_FORMAT("elf32-i386") */

ENTRY(arch_start)

SECTIONS {
	.code 0x100000 :
	{
		kernel_code = .;

		/* instructions */
		*?/arch/i386/arch/startup.asm.o ( .text* )

		*( .text* )
	}

	.data :
	{
		kernel_data = .;

		/* read only data (constants), initialized global variables */
		* ( .rodata* .data* )
	}

	.bss :
	{
		/* uninitialized global variables (or initialized with 0) */
		* ( .bss* COMMON* )

		. = ALIGN (4096);
	}

	k_kernel_end = .;

	/DISCARD/ : { *(.comment) } /* gcc info is discarded */
}
