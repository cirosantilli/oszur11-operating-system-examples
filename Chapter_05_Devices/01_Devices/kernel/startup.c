/*! Startup function - initialize kernel subsystem */
#define _KERNEL_

#include <arch/interrupts.h>
#include <kernel/time.h>
#include <kernel/devices.h>
#include <kernel/memory.h>
#include <kernel/kprint.h>
#include <kernel/errno.h>
#include <devices/console.h>
#include <arch/processor.h>
#include <lib/types.h>
#include <api/prog_info.h>

char system_info[] = 	OS_NAME ": " NAME_MAJOR ":" NAME_MINOR ", "
			"Version: " VERSION " (" PLATFORM ")";
//			"More info: " AUTHOR;

/* kernel (initial) stack */
uint8 k_stack [ STACK_SIZE ];

/* console for kernel messages */
console_t *k_stdout;

/* default standard input and output devices for user programs */
kdevice_t *_u_stdout;
extern void *u_stdout;

/*!
 * First kernel function (after grub loads it to memory)
 * \param magic	Multiboot magic number
 * \param addr	Address where multiboot structure is saved
 */
void k_startup ( unsigned long magic, unsigned long addr )
{
	extern console_t INITIAL_STDOUT; /* initial stdout */
	extern console_t STDOUT; /* default stdout */
	extern device_t STDOUT_DEV; /* default stdout as device */

	/* set initial stdout */
	k_stdout = &INITIAL_STDOUT;
	k_stdout->init( NULL );

	/* initialize memory subsystem (needed for boot) */
	k_memory_init ( magic, addr );

	/* start with regular initialization */

	/* interrupts */
	arch_init_interrupts ();

	/* timer subsystem */
	k_time_init ();

	/* devices */
	k_devices_init ();

	/* switch to default 'stdout' for kernel */
	k_stdout = &STDOUT;
	k_stdout->init( NULL );

	/* initialize 'stdout' for programs */
	u_stdout = _u_stdout = k_device_add ( &STDOUT_DEV );
	k_device_init ( _u_stdout, 0, NULL, NULL );

	kprint ( "%s\n", system_info );

	enable_interrupts ();

	/* "programs" - select programs to run */

	kprint ( "\nStarting program: hello_world\n\n" );
	hello_world ();

	kprint ( "\nStarting program: timer\n\n" );
	timer ();

	k_device_remove ( _u_stdout );

	kprint ( "\nSystem halted!" );
	halt();
}
