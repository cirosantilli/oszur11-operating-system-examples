/*! Interrupt handling - 'arch' layer (only basic operations) */

#pragma once

#ifndef ASM_FILE

/* (Hardware) Interrupt controller interface */
typedef struct _interrupt_controller_
{
	void (*init) ();
	void (*disable_irq) ( unsigned int irq );
	void (*enable_irq) ( unsigned int irq );
	void (*at_exit) ( unsigned int irq );

	char *(*int_descr) ( unsigned int irq );
}
arch_ic_t;

void arch_init_interrupts ();
void *arch_register_interrupt_handler ( unsigned int inum, void *handler );

extern void (*arch_irq_enable) ( unsigned int irq );
extern void (*arch_irq_disable) ( unsigned int irq );

#endif /* ASM_FILE */

/* Programmable Interrupt controllers (currently implemented only one, i8259) */
#include <arch/devices/i8259.h>

/* Constants */
#define SOFTWARE_INTERRUPT	SOFT_IRQ
#define INTERRUPTS		NUM_IRQS
