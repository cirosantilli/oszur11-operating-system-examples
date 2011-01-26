/*! Memory management */
#define _KERNEL_

#include "memory.h"
#include <arch/multiboot.h>
#include <arch/processor.h>
#include <kernel/kprint.h>
#include <kernel/errno.h>
#include <lib/string.h>
#include <lib/list.h>

/*! Memory map */
static mseg_t k_kernel;	/* kernel code and data */
static mseg_t k_heap;	/* kernel heap: for everything else */

static uint multiboot; /* save multiboot block address */

/*! Dynamic memory allocator for kernel */
MEM_ALLOC_T *k_mpool;

/*!
 * Init memory layout: using variables "from" linker script and multiboot info
 */
void k_memory_init ( unsigned long magic, unsigned long addr )
{
	extern char kernel_code, k_kernel_end;
	multiboot_info_t *mbi;

	/* implicitly from kernel linker script */
	k_kernel.start = &kernel_code;
	k_kernel.size = ( (uint) &k_kernel_end ) - (uint) &kernel_code;

	/* Am I booted by a Multiboot-compliant boot loader? */
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		kprint ( "Boot loader is not multiboot-compliant!\n" );
		halt();
	}

	/* from multiboot info */
	multiboot = addr;
	mbi = (void *) addr;

	k_heap.start = (void *) &k_kernel_end;
	k_heap.size = ( mbi->mem_upper - 1024 ) * 1024 - (size_t) &k_kernel_end;

	/* initialize dynamic memory allocation subsystem (needed for boot) */
	k_mpool = k_mem_init ( k_heap.start, k_heap.size );
}

/*! System information (and details) */
int sys__sysinfo ( char *buffer, size_t buf_size, char **param )
{
	char usage[] = "Usage: sysinfo [memory]";

	ASSERT_ERRNO_AND_EXIT ( buffer, E_PARAM_NULL );

	if ( param[1] == NULL )
	{
		/* only basic info */
		extern char system_info[];

		if ( strlen ( system_info ) > buf_size )
			EXIT ( E_TOO_BIG );

		strcpy ( buffer, system_info );

		EXIT ( SUCCESS );
	}
	else {
		/* extended info is requested */
		if ( strcmp ( "memory", param[1] ) == 0 )
		{
			k_memory_info ();
			buffer[0] = 0;
			EXIT ( SUCCESS );
		} /* TODO: "memory [segments|modules|***]" */
		else {
			if ( strlen ( usage ) > buf_size )
				EXIT ( E_TOO_BIG );
			strcpy ( buffer, usage );
			EXIT ( E_DONT_EXIST );
		}
	}
}

/*! print memory layout */
void k_memory_info ()
{
	multiboot_info_t *mbi = (void *) multiboot;

	kprint ( "MOOLTIBOOT info at %x flags=%x\n", mbi, mbi->flags );

	if ( mbi->flags & MULTIBOOT_INFO_MEMORY )
	{
		kprint ( "Available memory: low = %d kB, high = %d kB\n",
			  mbi->mem_lower, mbi->mem_upper );
	}

	if ( mbi->flags & MULTIBOOT_INFO_CMDLINE )
	{
		kprint ( "Command line: %s\n", (char *) mbi->cmdline );
	}

#if 1
	if ( mbi->flags & MULTIBOOT_INFO_MEM_MAP )
	{
		kprint ( "Memory map at %x (size=%d)\n",
			  mbi->mmap_addr, mbi->mmap_length );

		multiboot_memory_map_t *memmap = (void *) mbi->mmap_addr;
		int i;

		for ( i = 0; (uint32) memmap < mbi->mmap_addr + mbi->mmap_length;
			i++ )
		{
			kprint ( "* segment %d size=%x [%x - %x], flags= %d\n",
				  i, (uint32) memmap->len & 0xffffffff,
				  (uint32) memmap->addr & 0xffffffff,
				  ( (uint32) memmap->addr & 0xffffffff ) +
				  ( (uint32) memmap->len & 0xffffffff ) - 1,
				  memmap->type );
			memmap = (void *) ( (uint32) memmap + memmap->size +
				sizeof (memmap->size));
		}
	}
#endif
	kprint ( "MEMORY MAP (mmap):\n" );

	kprint ( "* Kernel code&data:  %x, size=%x\n",
		  k_kernel.start, k_kernel.size );

	kprint ( "* Kernel heap:       %x, size=%x",
		  k_heap.start, k_heap.size );
}
