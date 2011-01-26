/*! Startup function - initialize kernel subsystems */
#define _KERNEL_

#include <arch/interrupts.h>
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

static void test_interrupts ();
static void processor_irq_handler ( uint irq_num );
static void device_irq_handler ( uint irq_num );

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

	/* start with regular initialization */

	/* interrupts */
	arch_init_interrupts ();

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

	kprint ( "\nStarting program: hello_world\n\n" );
	hello_world ();

	kprint ( "\nStarting interrupt tests\n\n" );
	test_interrupts ();

	kprint ( "System halted!" );
	halt();
}

static void test_interrupts ()
{
	/* processor interrupts are usualy fatal - can't recover easy! */
	arch_register_interrupt_handler ( 0, processor_irq_handler );
	arch_register_interrupt_handler ( 5, processor_irq_handler );
	arch_register_interrupt_handler ( 10, processor_irq_handler );
	arch_register_interrupt_handler ( 15, processor_irq_handler );

	/* interrupts generated outside processor */
	arch_register_interrupt_handler ( 32, device_irq_handler );
	arch_register_interrupt_handler ( 35, device_irq_handler );
	arch_register_interrupt_handler ( 37, device_irq_handler );
	arch_register_interrupt_handler ( 39, device_irq_handler );

	/* irq number outside allowed range */
	//arch_register_interrupt_handler ( 60, device_irq_handler );

	/* raise a few interrupts (comment/uncomment for testing) */
	//raise_interrupt ( 0 );
	//raise_interrupt ( 1 );
	//raise_interrupt ( 5 );
	//raise_interrupt ( 7 );
	//raise_interrupt ( 10 );

	//raise_interrupt ( 20 );

	raise_interrupt ( 32 );
	raise_interrupt ( 35 );
	raise_interrupt ( 38 );
}

static void processor_irq_handler ( uint irq_num )
{
	kprint ( "Processor generated interrupt number: %d\n", irq_num );
	kprint ( "Halting!\n" );
	halt ();
}

static void device_irq_handler ( uint irq_num )
{
	kprint ( "Some device generated interrupt number: %d\n", irq_num );
	kprint ( "Continuing...\n" );
}
