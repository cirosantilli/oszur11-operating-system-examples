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
uint32 arch_thr_context_ss;
uint32 *arch_thr_context;

/*!
 * Set up context (normal and interrupt=kernel)
 */
void arch_context_init ()
{
	arch_thr_context_ss = GDT_DESCRIPTOR ( SEGM_K_DATA, GDT, PRIV_KERNEL );

	arch_interrupt_stack = (void *) &k_stack [ KERNEL_STACK_SIZE ];

	arch_descriptors_init (); /* GDT, IDT, ... */
}

/*! context manipulation ---------------------------------------------------- */

/*! Create initial context for thread - it should start with defined function
   (context is identical to interrupt frame - use same code to start/return) */
void arch_create_thread_context ( context_t *context,
		void (func) (void *), void *param, void (*thread_exit)(),
		void *stack, size_t stack_size, void *proc )
{
	/* thread stack */
	context->context.esp = stack + stack_size;
	/* put starting thread function parameter on stack */
	*( --context->context.esp ) = (uint32) param;
	/* return address (when thread exits */
	*( --context->context.esp ) = (uint32) thread_exit;

	/* interrupt frame */
	context->context.eflags = INIT_EFLAGS;
	context->context.cs = GDT_DESCRIPTOR ( SEGM_T_CODE, GDT, PRIV_USER );
	context->context.eip = (uint32) func;

	context->context.ss = context->context.ds = context->context.es =
	context->context.fs = context->context.gs = context->context.ss =
		GDT_DESCRIPTOR ( SEGM_T_DATA, GDT, PRIV_USER );

	/* rest of context is not relevant for new thread */
#ifdef DEBUG
	context->context.err = 0;
	context->context.eax = context->context.ecx = context->context.edx =
	context->context.ebx = context->context.ebp = context->context.esi =
	context->context.edi = 0;
#endif
	context->proc = proc;

	context->context.esp = K2U_GET_ADR ( context->context.esp, proc );
	/* stack pointer (as eip) must be in process relative addresses */
}

/*! Select thread to return to from interrupt */
inline void arch_select_thread ( context_t *context )
{
	arch_thr_context = (void *) &context->context;
	arch_tss_update(((void *) &context->context) + sizeof (arch_context_t));

	/* update segment descriptors */
	arch_update_user_segments ( k_process_start_adr ( context->proc ),
				    k_process_size ( context->proc ) );
}
