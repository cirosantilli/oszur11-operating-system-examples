/*! Processor context, segment descriptor tables: GDT, IDT */

#define _ARCH_CONTEXT_C_
#include "context.h"

#include <arch/interrupts.h>
#include <arch/descriptors.h>
#include <kernel/memory.h>

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

	/* thread context */
	context->context->eip = (uint32) func;
	context->context->eflags = INIT_EFLAGS;

	/* rest of context is not relevant for new thread */
}

/* switch from one thread to another */
void arch_switch_to_thread ( context_t *from, context_t *to )
{
	asm volatile (
			"pushl	$1f		\n\t"	/* EIP */
			"pushfl			\n\t"	/* EFLAGS */
			"pushal			\n\t"	/* all registers */
			"movl	%%esp, %0	\n\t"	/* save stack => from */

			"movl	%1, %%esp	\n\t"	/* restore stack <= to*/
			"popal			\n\t"
			"popfl			\n\t"
			"ret			\n\t"

		"1:	 nop			\n\t"

			: "=m" (from->context): "m" (to->context)
	);
}
