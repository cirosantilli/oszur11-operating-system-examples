/*! Processor/thread context */

#pragma once

#include <lib/types.h>

/*! context manipulation - for 'kernel threads'  ---------------------------- */

#define	INIT_EFLAGS	0x0202 /* ring 0 ! */

/*! stack after context switch (for saved thread) */
typedef struct _arch_context_t_
{
	int32 edi, esi, ebp, _esp, ebx, edx, ecx, eax;
	uint32 eflags;
	uint32 eip;
}
__attribute__((__packed__)) arch_context_t;

/*! Thread context */
typedef struct _context_t_
{
	arch_context_t *context; /* thread context is on stack! */
}
context_t;

void arch_create_thread_context ( context_t *context,
		void (func) (void *), void *param, void (*thread_exit)(),
		void *stack, size_t stack_size );

void arch_switch_to_thread ( context_t *from, context_t *to );
