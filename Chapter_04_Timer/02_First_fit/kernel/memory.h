/*! Memory management */

#pragma once

#include <lib/types.h>

/*! Dynamic memory --------------------------------------------------- */
#include <lib/ff_simple.h>

#define	k_mem_init(segment, size)	ffs_init ( segment, size )
#define	kmalloc(size)			ffs_alloc ( k_mpool, size )
#define	kfree(addr)			ffs_free ( k_mpool, addr )

extern ffs_mpool_t *k_mpool;

#ifdef _KERNEL_

/*! Kernel memory layout ---------------------------------------------------- */

void k_memory_init ( unsigned long magic, unsigned long addr );

#endif /* _KERNEL_ */
