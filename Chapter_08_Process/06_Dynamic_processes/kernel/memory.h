/*! Memory management */

#pragma once


/*! Kernel dynamic memory --------------------------------------------------- */
#include <lib/mm/ff_simple.h>
#include <lib/mm/gma.h>

#if MEM_ALLOCATOR_FOR_KERNEL == FIRST_FIT

#define MEM_ALLOC_T ffs_mpool_t

#define	k_mem_init(segment, size)	ffs_init ( segment, size )
#define	kmalloc(size)			ffs_alloc ( k_mpool, size )
#define	kfree(addr)			ffs_free ( k_mpool, addr )

#elif MEM_ALLOCATOR_FOR_KERNEL == GMA

#define MEM_ALLOC_T gma_t

#define	k_mem_init(segment, size)	gma_init ( segment, size, 32, 0 )
#define	kmalloc(size)			gma_alloc ( k_mpool, size )
#define	kfree(addr)			gma_free ( k_mpool, addr )

#else /* memory allocator not selected! */

#error	Dynamic memory manager not defined!

#endif

extern MEM_ALLOC_T *k_mpool;


/*! Kernel memory layout ---------------------------------------------------- */
#include <lib/types.h>
#include <lib/list.h>
#include <api/prog_info.h>

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

/* Program, loaded as module */
typedef struct _kprog_t_
{
	char *prog_name; /* read from multiboot */

	prog_info_t *pi; /* defined as header of program */

	mseg_t m;

	list_h all;
}
kprog_t;

/*! Process ----------------------------------------------------------------- */

/* Process (programs loaded as modules) */
typedef struct _kprocess_t_
{
	kprog_t *prog;

	ffs_mpool_t *stack_pool;

	prog_info_t *pi; /* process header (copy of program header) */
	mseg_t m;

	int thr_count;

	list_h all;
}
kprocess_t;

static inline void *k_process_start_adr ( void *proc )
{
	return ( (kprocess_t *) proc )->m.start;
}

static inline size_t k_process_size ( void *proc )
{
	return ( (kprocess_t *) proc )->m.size;
}

/* -------------------------------------------------------------------------- */
/*! kernel <--> user address translation (with segmentation) */

extern inline void *k_u2k_adr ( void *uadr, kprocess_t *proc );
extern inline void *k_k2u_adr ( void *kadr, kprocess_t *proc );

#define U2K_GET_ADR(ADR,PROC)	k_u2k_adr (ADR, PROC)
#define U2K_GET_INT(ADR,PROC)	( *( (int *) k_u2k_adr (ADR, PROC) ) )

#define K2U_GET_ADR(ADR,PROC)	k_k2u_adr (ADR, PROC)

uint k_new_unique_id ();
void k_free_unique_id ( uint id );

int sys__sysinfo ( void *p );
int k_list_programs ( char *buffer, size_t buf_size );
