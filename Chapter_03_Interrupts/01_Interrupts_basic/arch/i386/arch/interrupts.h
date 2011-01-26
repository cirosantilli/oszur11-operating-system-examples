/*! Interrupt handling - 'arch' layer (only basic operations) */

#pragma once

void arch_init_interrupts ();
void *arch_register_interrupt_handler ( unsigned int inum, void *handler );

/* Constants */
#define SOFTWARE_INTERRUPT	48
#define INTERRUPTS		49
