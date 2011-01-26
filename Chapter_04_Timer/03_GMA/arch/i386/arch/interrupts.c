/*! Interrupt handling - 'arch' layer (only basic operations) */

#define _ARCH_INTERRUPTS_C_
#define _KERNEL_
#include "interrupts.h"

#include <arch/io.h>
#include <arch/processor.h>
#include <kernel/errno.h>
#include <lib/string.h>

/* Interrupt controller device */
extern arch_ic_t IC_DEV;
static arch_ic_t *icdev = &IC_DEV;

void (*arch_irq_enable) ( unsigned int );
void (*arch_irq_disable) ( unsigned int );

/*! interrupt handlers */
static int (*ihandler[INTERRUPTS]) ( unsigned int );

/*!
 * Initialize interrupt subsystem (in 'arch' layer)
 */
void arch_init_interrupts ()
{
	icdev->init ();

	arch_irq_enable = icdev->enable_irq;
	arch_irq_disable = icdev->disable_irq;

	memset ( ihandler, 0, sizeof(ihandler) );
}

/*!
 * Register handler function for particular interrupt number
 */
void *arch_register_interrupt_handler ( unsigned int inum, void *handler )
{
	void *old_handler = NULL;

	if ( inum < INTERRUPTS )
	{
		old_handler = ihandler[inum];
		ihandler[inum] = handler;
	}
	else {
		LOG ( ERROR, "Interrupt %d can't be used!\n", inum );
		halt ();
	}

	return old_handler;
}

/*!
 * "Forward" interrupt handling to registered handler
 * (called from interrupts.S)
 */
void arch_interrupt_handler ( int irq_num )
{
	if ( irq_num < INTERRUPTS && ihandler[irq_num] != NULL )
	{
		/*! Call registered handler */
		ihandler[irq_num] ( irq_num );

		if ( icdev->at_exit )
			icdev->at_exit ( irq_num );
	}

	else if ( irq_num < INTERRUPTS )
	{
		LOG ( ERROR, "Unregistered interrupt: %d - %s!\n",
		      irq_num, icdev->int_descr ( irq_num ) );
		halt ();
	}
	else {
		LOG ( ERROR, "Unregistered interrupt: %d !\n", irq_num );
		halt ();
	}
}
