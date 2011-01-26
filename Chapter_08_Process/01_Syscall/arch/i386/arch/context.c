/*! Processor context, segment descriptor tables: GDT, IDT */

#define _ARCH_CONTEXT_C_
#include "context.h"

#include <arch/interrupts.h>
#include <arch/descriptors.h>
#include <kernel/memory.h>

/* kernel (interrupt) stack */
uint8 k_stack [ KERNEL_STACK_SIZE ];

/* interrupt handler stack */
void *arch_interrupt_stack;

/* where is thread context saved at interrupt? */
uint32 *arch_thr_context;

/*!
 * Set up context (normal and interrupt=kernel)
 */
void arch_context_init ()
{
	arch_interrupt_stack = (void *) &k_stack [ KERNEL_STACK_SIZE ];

	arch_descriptors_init (); /* GDT, IDT, ... */
}

/*! context manipulation ---------------------------------------------------- */

/*! Create initial context for thread - it should start with defined function
   (context is identical to interrupt frame - use same code to start/return) */
void arch_create_thread_context ( context_t *context,
		void (func) (void *), void *param, void (*thread_exit)(),
		void *stack, size_t stack_size )
{
	uint32 *tstack;

	/* thread stack */
	tstack = stack + stack_size;

	/* put starting thread function parameter on stack */
	*( --tstack ) = (uint32) param;
	/* return address (when thread exits */
	*( --tstack ) = (uint32) thread_exit;

	/* thread context is on stack */
	context->context = (void *) tstack - sizeof (arch_context_t);

	/* interrupt frame */
	context->context->eflags = INIT_EFLAGS;
	context->context->cs = GDT_DESCRIPTOR ( SEGM_CODE, GDT, PRIV_KERNEL );
	context->context->eip = (uint32) func;

	/* rest of context is not relevant for new thread */
}

/*! Select thread to return to from interrupt */
inline void arch_select_thread ( context_t *context )
{
	arch_thr_context = (void *) context->context;
}
