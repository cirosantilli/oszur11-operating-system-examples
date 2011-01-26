/*! Startup function - initialize kernel subsystems */
#define _KERNEL_

#include <kernel/kprint.h>
#include <arch/console.h>
#include <arch/processor.h>
#include <lib/types.h>
#include <api/prog_info.h>

char system_info[] = 	OS_NAME ": " NAME_MAJOR ":" NAME_MINOR ", "
			"Version: " VERSION " (" PLATFORM ")";
//			"More info: " AUTHOR;

/* kernel (initial) stack */
uint8 k_stack [ STACK_SIZE ];

/*!
 * First kernel function (after grub loads it to memory)
 * \param magic	Multiboot magic number
 * \param addr	Address where multiboot structure is saved
 */
void k_startup ( unsigned long magic, unsigned long addr )
{
	arch_console_init ();

	kprint ( "%s\n", system_info );

	/* "programs" - select programs to run */
	kprint ( "\nStarting program: hello_world\n\n" );
	hello_world ();

	kprint ( "\nSystem halted!" );
	halt ();
}
