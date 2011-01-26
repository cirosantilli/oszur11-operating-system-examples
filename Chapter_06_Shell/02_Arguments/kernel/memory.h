/*! Memory management */

#pragma once

#include <lib/types.h>

/*! syscall wrappers -------------------------------------------------------- */
int sys__sysinfo ( char *buffer, size_t buf_size, char **param );

/*! Dynamic memory --------------------------------------------------- */
#include <lib/mm/ff_simple.h>
#include <lib/mm/gma.h>

#if MEM_ALLOCATOR == FIRST_FIT

#define MEM_ALLOC_T ffs_mpool_t

#define	k_mem_init(segment, size)	ffs_init ( segment, size )
#define	kmalloc(size)			ffs_alloc ( k_mpool, size )
#define	kfree(addr)			ffs_free ( k_mpool, addr )

#elif MEM_ALLOCATOR == GMA

#define MEM_ALLOC_T gma_t

#define	k_mem_init(segment, size)	gma_init ( segment, size, 32, 0 )
#define	kmalloc(size)			gma_alloc ( k_mpool, size )
#define	kfree(addr)			gma_free ( k_mpool, addr )

#else /* memory allocator not selected! */

#error	Dynamic memory manager not defined!

#endif

extern MEM_ALLOC_T *k_mpool;


#ifdef _KERNEL_

/*! Kernel memory layout ---------------------------------------------------- */

/* Memory segment */
typedef struct _mseg_t_
{
	void *start;
	size_t size;
}
mseg_t;

#define ALIGN_TO	4096 /* align segments to it */

void k_memory_init ( unsigned long magic, unsigned long addr );
void k_memory_info ();

#endif /* _KERNEL_ */
