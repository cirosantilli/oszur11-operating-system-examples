/*! Startup function - initialize kernel subsystem */
#define _KERNEL_

#include <arch/interrupts.h>
#include <kernel/time.h>
#include <kernel/thread.h>
#include <kernel/syscall.h>
#include <kernel/devices.h>
#include <kernel/memory.h>
#include <kernel/kprint.h>
#include <kernel/errno.h>
#include <arch/processor.h>
#include <lib/types.h>

char system_info[] = 	OS_NAME ": " NAME_MAJOR ":" NAME_MINOR ", "
			"Version: " VERSION " (" PLATFORM ")";
//			"More info: " AUTHOR;

/* default standard input and output devices for user programs (threads) */
void *u_stdin, *u_stdout;

/*!
 * First kernel function (after grub loads it to memory)
 * \param magic	Multiboot magic number
 * \param addr	Address where multiboot structure is saved
 */
void k_startup ( unsigned long magic, unsigned long addr )
{
	extern device_t K_INITIAL_STDOUT;
	extern kdevice_t *k_stdout; /* console for kernel messages */
	kdevice_t k_initial_stdout;
	/* default standard input and output devices for user programs */

	/* set initial stdout */
	k_initial_stdout.dev = K_INITIAL_STDOUT;
	k_initial_stdout.dev.init ( 0, NULL, &k_initial_stdout.dev );
	k_stdout = &k_initial_stdout;

	/* initialize memory subsystem (needed for boot) */
	k_memory_init ( magic, addr );

	/*! start with regular initialization */

	/* interrupts */
	arch_init_interrupts ();
	arch_register_interrupt_handler ( SOFTWARE_INTERRUPT, k_syscall, NULL );

	/* timer subsystem */
	k_time_init ();

	/* devices */
	k_devices_init ();

	/* switch to default 'stdout' for kernel */
	k_stdout = k_device_open ( K_STDOUT );
	u_stdin = k_device_open ( U_STDIN );
	u_stdout = k_device_open ( U_STDOUT );

	kprint ( "%s\n", system_info );

	/* thread subsystem */
	k_threads_init ();

	/* complete initialization by starting first thread */
	arch_return_to_thread ();
}
