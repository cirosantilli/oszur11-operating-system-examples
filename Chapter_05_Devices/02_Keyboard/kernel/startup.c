/*! Startup function - initialize kernel subsystem */
#define _KERNEL_

#include <arch/interrupts.h>
#include <kernel/time.h>
#include <kernel/devices.h>
#include <kernel/memory.h>
#include <kernel/kprint.h>
#include <kernel/errno.h>
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
	extern device_t K_INITIAL_STDOUT; /* initial kernel stdout */
	extern device_t K_STDOUT; /* default kernel stdout */
	extern device_t U_STDOUT; /* default user stdout */
	extern device_t U_STDIN; /* default stdin */
	extern kdevice_t *k_stdout; /* console for kernel messages */
	kdevice_t k_initial_stdout;
	/* default standard input and output devices for user programs */
	kdevice_t *_u_stdin, *_u_stdout;
	extern void *u_stdin, *u_stdout;

	/* set initial stdout */
	k_initial_stdout.dev = K_INITIAL_STDOUT;
	k_initial_stdout.dev.init ( 0, NULL, &k_initial_stdout.dev );
	k_stdout = &k_initial_stdout;

	/* initialize memory subsystem (needed for boot) */
	k_memory_init ( magic, addr );

	/*! start with regular initialization */

	/* interrupts */
	arch_init_interrupts ();

	/* timer subsystem */
	k_time_init ();

	/* devices */
	k_devices_init ();

	/* switch to default 'stdout' for kernel */
	k_stdout = k_device_add ( &K_STDOUT );
	k_device_init ( k_stdout, 0, NULL, NULL );

	/* initialize 'stdout' and 'stdin' for programs */
	u_stdin = _u_stdin = k_device_add ( &U_STDIN );
	k_device_init ( _u_stdin, ECHO_OFF, NULL, NULL );

	if ( &U_STDIN != &U_STDOUT )
	{
		u_stdout = _u_stdout = k_device_add ( &U_STDOUT );
		k_device_init ( u_stdout, 0, NULL, NULL );
	}
	else {
		u_stdout = _u_stdout = _u_stdin;
	}

	kprint ( "%s\n", system_info );

	enable_interrupts ();

	/* "programs" - select programs to run */

	kprint ( "Starting program: hello_world\n" );
	hello_world ();

	kprint ( "Starting program: timer\n" );
	timer ();

	kprint ( "Starting program: keyboard\n" );
	keyboard ();

	k_device_remove ( _u_stdin );
	if ( _u_stdout != _u_stdin )
		k_device_remove ( _u_stdout );

	kprint ( "\nSystem halted!\n" );

	k_device_remove ( k_stdout );

	halt();
}
