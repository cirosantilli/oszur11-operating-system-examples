/*! Memory management */
#define _KERNEL_

#include "memory.h"
#include <arch/multiboot.h>
#include <arch/processor.h>
#include <kernel/kprint.h>

/*! Dynamic memory allocator for kernel */
ffs_mpool_t *k_mpool;

/*!
 * Init memory layout: using variables "from" linker script and multiboot info
 */
void k_memory_init ( unsigned long magic, unsigned long addr )
{
	extern char k_kernel_end;
	multiboot_info_t *mbi;
	void *mem_for_mpool;
	size_t mpool_size;

	/* Am I booted by a Multiboot-compliant boot loader? */
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		kprint ( "Boot loader is not multiboot-compliant!\n" );
		halt();
	}

	/* from multiboot info */
	mbi = (void *) addr;

	mem_for_mpool = (void *) &k_kernel_end;
	mpool_size = ( mbi->mem_upper - 1024 ) * 1024 - (size_t) mem_for_mpool;

	/* initialize dynamic memory allocation subsystem */
	k_mpool = k_mem_init ( mem_for_mpool, mpool_size );
}
