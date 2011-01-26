/*! Startup function - initialize kernel subsystems */
#define _KERNEL_

#include <arch/interrupts.h>
#include <kernel/time.h>
#include <kernel/memory.h>
#include <kernel/kprint.h>
#include <kernel/errno.h>
#include <arch/console.h>
#include <arch/processor.h>
#include <lib/types.h>
#include <api/prog_info.h>

char system_info[] = 	OS_NAME ": " NAME_MAJOR ":" NAME_MINOR ", "
			"Version: " VERSION " (" PLATFORM ")";
//			"More info: " AUTHOR;

/* kernel (initial) stack */
uint8 k_stack [ STACK_SIZE ];

console_t *k_stdout; /* console for kernel messages */
console_t *u_stdout; /* default standard output for user programs */

/*!
 * First kernel function (after grub loads it to memory)
 * \param magic	Multiboot magic number
 * \param addr	Address where multiboot structure is saved
 */
void k_startup ( unsigned long magic, unsigned long addr )
{
	extern console_t INITIAL_STDOUT; /* initial stdout */
	extern console_t STDOUT; /* default stdout for kernel */
	extern console_t STDOUT_PROG; /* default stdout for programs */

	/* set initial stdout */
	k_stdout = &INITIAL_STDOUT;
	k_stdout->init ( NULL );

	/* initialize memory subsystem (needed for boot) */
	k_memory_init ( magic, addr );

	/* start with regular initialization */

	/* interrupts */
	arch_init_interrupts ();

	/* timer subsystem */
	k_time_init ();

	/* switch to default 'stdout' for kernel */
	//extern console_t dev_null;
	//k_stdout = &dev_null;
	k_stdout = &STDOUT;
	k_stdout->init( NULL );

	/* initialize 'stdout' for programs */
	u_stdout = &STDOUT_PROG;
	u_stdout->init ( NULL );

	kprint ( "%s\n", system_info );

	enable_interrupts ();

	/* "programs" - select programs to run */

//	kprint ( "\nStarting program: hello_world\n\n" );
//	hello_world ();

//	kprint ( "\nStarting program: timer\n\n" );
//	timer ();

	kprint ( "\nStarting program: dmem\n\n" );
	dmem ();

	kprint ( "System halted!" );
	halt();
}
