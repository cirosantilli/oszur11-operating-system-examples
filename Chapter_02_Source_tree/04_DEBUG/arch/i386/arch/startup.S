/*! startup.S - starting point of control after grub (boot loader) */

#define ASM_FILE        1

#include <arch/multiboot.h>

/* stack, startup function */
.extern	k_stack, k_startup

/* this code must be first in image for grub to find it easy */
.section .text

/* entry point (required for grub) */
.global arch_start

/* 32 bit alignment is required for following constants */
.align  4

/* Multiboot header */
multiboot_header:
	/* magic */
	.long   MULTIBOOT_HEADER_MAGIC
	/* flags */
	.long   MULTIBOOT_HEADER_FLAGS
	/* checksum */
	.long   -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

/* THE starting point */
arch_start:
	/* stack pointer initialization */
	mov    $(k_stack + STACK_SIZE), %esp

	/* starting status register - EFLAGS register */
	pushl   $0
	popf

	/* copy multiboot parameters on stack as parameters to 'k_startup'
	   (parameters are already in eax:ebx registers), set by grub) */
	push   %ebx
	push   %eax

	call    k_startup

	/* stop */
	cli
loop:   hlt
	jmp     loop
