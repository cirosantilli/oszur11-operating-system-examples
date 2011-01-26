/*! Processor/thread context */

#pragma once

#include <lib/types.h>

/*! context manipulation - for 'kernel threads'  ---------------------------- */

#define	INIT_EFLAGS	0x0202 /* ring 0 ! */

typedef struct _arch_context_t_
{
	int32 edi, esi, ebp, _esp, ebx, edx, ecx, eax;
	int32 err;
	uint32 eip;
	uint32 cs;
	uint32 eflags;
}
__attribute__((__packed__)) arch_context_t;

typedef struct _context_t_
{
	arch_context_t *context; /* thread context is on stack! */
}
context_t;

void arch_create_thread_context ( context_t *context,
		void (func) (void *), void *param, void (*thread_exit)(),
		void *stack, size_t stack_size );
extern inline void arch_save_thread ( context_t *cntx );
extern inline void arch_select_thread ( context_t *cntx );

void arch_switch_to_thread ( context_t *from, context_t *to );

void arch_dump_thread_context ( context_t *cntx );


/*! context manipulation - for 'user threads' (in programs) ----------------- */

typedef struct _ucontext_t_
{
	uint32 *esp;
}
ucontext_t;

#define CONTEXT_SIZE	( sizeof(int) * 10 )

/*! context manipulation - for 'user managed threads' (in programs) --------- */

static inline void arch_create_uthread_context ( ucontext_t *context,
		void (func) (void *), void *param, void (thread_exit)(),
		void *stack, size_t stack_size )
{
	context->esp = stack + stack_size;

	*( --context->esp ) = (uint32) param;
	*( --context->esp ) = (uint32) thread_exit;
	*( --context->esp ) = (uint32) func;

	*( --context->esp ) = INIT_EFLAGS;	/* EFLAGS register */

	/* set initial values for all general purpose registers (8) on stack */
	*( --context->esp ) = 0;	/* EAX */
	*( --context->esp ) = 0;	/* ECX */
	*( --context->esp ) = 0;	/* EDX */
	*( --context->esp ) = 0;	/* EBX */
	*( --context->esp ) = 0;	/* ESP */
	*( --context->esp ) = 0;	/* EBP */
	*( --context->esp ) = 0;	/* ESI */
	*( --context->esp ) = 0;	/* EDI */

	/* stack (context->esp) is now ready for 'context_restore' function */
}

static inline void arch_switch_to_uthread ( ucontext_t *from, ucontext_t *to )
{
	asm volatile (
		"	pushl $1f\n"
		"	pushfl\n"
		"	pushal\n"
		"	mov %%esp, %0\n"

		"	mov %1, %%esp\n"

		"	popal\n"
		"	popfl\n"
		"	ret\n"
		"1:	nop\n"

		: "=m" (from->esp) : "m" (to->esp)
	);
}
