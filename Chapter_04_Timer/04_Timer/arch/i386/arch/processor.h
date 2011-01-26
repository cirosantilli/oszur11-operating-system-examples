/*! Assembler macros for some processor control instructions */

#pragma once

#define disable_interrupts()	asm volatile ( "cli\n\t" )
#define enable_interrupts()	asm volatile ( "sti\n\t" )

#define halt()			asm volatile ( "cli \n\t" "hlt \n\t" );

#define suspend()		asm volatile ( "hlt \n\t" );

#define raise_interrupt(p)	asm volatile ("int %0\n\t" :: "i" (p):"memory")

#define memory_barrier()	asm ("" : : : "memory")
