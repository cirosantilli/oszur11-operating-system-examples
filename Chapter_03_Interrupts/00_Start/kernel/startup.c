/*! Startup function - initialize kernel subsystems */
#define _KERNEL_

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

#ifdef DEBUG
static int debug_output_test ();
#endif

/*!
 * First kernel function (after grub loads it to memory)
 * \param magic	Multiboot magic number
 * \param addr	Address where multiboot structure is saved
 */
void k_startup ( unsigned long magic, unsigned long addr )
{
	extern console_t STDOUT; /* default stdout for kernel */
	extern console_t STDOUT_PROG; /* default stdout for programs */

	/* initialize 'stdout' for kernel */
	k_stdout = &STDOUT;
	/* or, for example: extern console_t dev_null; k_stdout = &dev_null; */

	k_stdout->init ( NULL );

	/* initialize 'stdout' for programs */
	u_stdout = &STDOUT_PROG;
	u_stdout->init ( NULL );

	kprint ( "%s\n", system_info );

	/* "programs" - select programs to run */
	kprint ( "\nStarting program: hello_world\n\n" );
	hello_world ();

#ifdef DEBUG
	kprint ( "\nLOG, ASSERT* test\n" );
	debug_output_test ();
#endif
	kprint ( "\nSystem halted!" );
	halt ();
}

#ifdef DEBUG
static int debug_output_test ()
{
	LOG ( WARN, "This is log entry with WARN relevance" );
	LOG ( INFO, "Address of u_stdout is %x", u_stdout );

	//ASSERT_ERRNO_AND_EXIT ( u_stdout, E_PARAM_NULL );
	//ASSERT_ERRNO_AND_EXIT ( !u_stdout, E_UNKOWN );

	//ASSERT ( u_stdout );
	//ASSERT ( !u_stdout );

	return 0;
}
#endif
