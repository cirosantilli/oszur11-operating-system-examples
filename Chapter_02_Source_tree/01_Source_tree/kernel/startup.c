/*! Startup function */

#include <arch/print.h>
#include <lib/types.h>
#include <api/prog_info.h>

/* kernel (initial) stack */
uint8 k_stack [ STACK_SIZE ];

/*! First kernel function (after grub loads it to memory) */
void k_startup ()
{
	arch_console_init ();

	hello_world ();
}
