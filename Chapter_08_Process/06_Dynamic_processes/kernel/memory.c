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

/*! List of programs loaded as modules */
list_t progs;
#define PNAME "prog_name="

/*!
 * Init memory layout: using variables "from" linker script and multiboot info
 */
void k_memory_init ( unsigned long magic, unsigned long addr )
{
	extern char kernel_code, k_kernel_end;
	multiboot_info_t *mbi;
	multiboot_module_t *mod;
	uint max;
	int i;
	kprog_t *prog;
	char *name, *pos;

	/* implicitly from kernel linker script */
	k_kernel.start = &kernel_code;
	k_kernel.size = ( (uint) &k_kernel_end ) - (uint) &kernel_code;

	/* Am I booted by a Multiboot-compliant boot loader? */
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		kprint ( "Boot loader is not multiboot-compliant!\n" );
		halt();
	}

	list_init ( &progs );

	/* from multiboot info */
	multiboot = addr;
	mbi = (void *) addr;

	max = (uint) &k_kernel_end;

	/* first run on modules - check size */
	if ( mbi->flags & MULTIBOOT_INFO_MODS )
	{
		mod = (void *) mbi->mods_addr;
		for ( i = 0; i < mbi->mods_count; i++, mod++ )
			if ( max < mod->mod_end )
				max = mod->mod_end + 1;
	}

	if ( max % ALIGN_TO )
		max += ALIGN_TO - ( max % ALIGN_TO );

	k_heap.start = (void *) max;
	k_heap.size = ( mbi->mem_upper - 1024 ) * 1024 - max;

	/* initialize dynamic memory allocation subsystem (needed for boot) */
	k_mpool = k_mem_init ( k_heap.start, k_heap.size );

	/* second run on modules - initialize them */
	if ( mbi->flags & MULTIBOOT_INFO_MODS )
	{
		mod = (void *) mbi->mods_addr;

		for ( i = 0; i < mbi->mods_count; i++, mod++ )
		{
			/* Is this module a program?
			   Programs must have 'prog_name' in command line */
			name = strstr ( (char *) mod->cmdline, PNAME );
			if ( name )
			{
				name += strlen ( PNAME );
				pos = strchr ( name, ' ' );
				if ( pos )
					*pos++ = 0;

				prog = kmalloc ( sizeof (kprog_t) );
				prog->prog_name = name;
				prog->pi = (void *) mod->mod_start;

				prog->m.start = prog->pi;
				prog->m.size = (size_t) prog->pi->end_adr -
					       (size_t) prog->pi->start_adr;

				list_append ( &progs, prog, &prog->all );
			}
		}
	}
}

/*! kernel <--> user address translation (using segmentation) */
inline void *k_u2k_adr ( void *uadr, kprocess_t *proc )
{
	if ( (aint) uadr >= proc->m.size )
		kprint ( "addr:%x size:%x\n", uadr, proc->m.size );
	ASSERT ( (aint) uadr < proc->m.size );

	return uadr + (aint) proc->m.start;
}
inline void *k_k2u_adr ( void *kadr, kprocess_t *proc )
{
	ASSERT ( (aint) kadr >= (aint) proc->m.start &&
		 (aint) kadr < (aint) proc->m.start + proc->m.size );

	return kadr - (aint) proc->m.start;
}

/* unique system wide id */
#define WBITS	( sizeof(word_t) * 8 )
static word_t idmask[ MAX_RESOURCES / WBITS ] = { 0 };
static uint last_id = 0;

/*! Allocate and return unique id for new thread */
uint k_new_unique_id ()
{
	uint starting = last_id;

	do {
		last_id = ( last_id + 1 ) % MAX_RESOURCES;
		if ( last_id == starting )
		{
			LOG ( ERROR, "Don't have free unique id!\n" );
			halt();
		}
		if ( last_id == 0 ) /* don't assign 0 */
			continue;
	}
	while ( idmask [ last_id / WBITS ] & ( 1 << ( last_id % WBITS ) ) );

	idmask [ last_id / WBITS ] |= 1 << ( last_id % WBITS );

	return last_id;
}

/*! Release thread id (after corresponding threads is removed from system) */
void k_free_unique_id ( uint id )
{
	ASSERT ( id > 0 && id < MAX_RESOURCES &&
		 ( idmask [ id / WBITS ] & ( 1 << ( id % WBITS ) ) ) );

	idmask [ id / WBITS ] &= ~ ( 1 << ( id % WBITS ) );
}


/*! System information (and details) */
int sys__sysinfo ( void *p )
{
	char *buffer;
	size_t buf_size;
	char **param; /* last param is NULL */
	char *param0, *param1;
	char usage[] = "Usage: sysinfo [programs|threads|memory]";

	buffer = *( (char **) p ); p += sizeof (char *);
	ASSERT_ERRNO_AND_EXIT ( buffer, E_PARAM_NULL );

	buffer = U2K_GET_ADR ( buffer, k_get_active_process() );

	buf_size = *( (size_t *) p ); p += sizeof (size_t *);

	param = *( (char ***) p );
	param = U2K_GET_ADR ( param, k_get_active_process() );
	param0 = U2K_GET_ADR ( param[0], k_get_active_process() );

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
		param1 = U2K_GET_ADR ( param[1], k_get_active_process() );

		/* extended info is requested */
		if ( strcmp ( "programs", param1 ) == 0 )
		{
			EXIT ( k_list_programs ( buffer, buf_size ) );
		} /* TODO: "program prog_name" => print help_msg */
		else if ( strcmp ( "memory", param1 ) == 0 )
		{
			k_memory_info ();
			buffer[0] = 0;
			EXIT ( SUCCESS );
		} /* TODO: "memory [segments|modules|***]" */
		else if ( strcmp ( "threads", param1 ) == 0 )
		{
			k_thread_info ();
			buffer[0] = 0;
			EXIT ( SUCCESS );
		} /* TODO: "thread thr_id" */
		else {
			if ( strlen ( usage ) > buf_size )
				EXIT ( E_TOO_BIG );
			strcpy ( buffer, usage );
			EXIT ( E_DONT_EXIST );
		}
	}
}

/*!
 * Give list of all programs
 * \param buffer Pointer to string where to save all programs names
 * \param buf_size Size of 'buffer'
 * \returns 0 if successful, E_TOO_BIG if buffer not big enough
 */
int k_list_programs ( char *buffer, size_t buf_size )
{
	size_t cur_size;
	kprog_t *prog;
	char hdr[] = "List of programs:\n";

	buffer[0] = 0; /* set empty string */
	cur_size = 0;
	prog = list_get ( &progs, FIRST );

	if ( strlen ( hdr ) > buf_size )
		RETURN ( E_TOO_BIG );
	strcpy ( buffer, hdr );

	while ( prog )
	{
		cur_size += strlen ( prog->prog_name );

		if ( cur_size > buf_size )
			RETURN ( E_TOO_BIG );

		strcat ( buffer, prog->prog_name );
		strcat ( buffer, " " );
		prog = list_get_next ( &prog->all );
	}

	RETURN ( SUCCESS );
}

/*! print memory layout */
void k_memory_info ()
{
	multiboot_info_t *mbi = (void *) multiboot;
	multiboot_module_t *mod;
	int i;
	char *name, *pos;

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

	if ( mbi->flags & MULTIBOOT_INFO_MODS )
	{
		kprint ( "Modules: count %d, at %x\n",
			  mbi->mods_count, mbi->mods_addr );

		mod = (void *) mbi->mods_addr;

		for ( i = 0; i < mbi->mods_count; i++ )
		{
			/* is it program? must have prog_name in command line */
			name = strstr ( (char *) mod->cmdline, PNAME );
			if ( name )
			{
				name += strlen ( PNAME );
				pos = strchr ( name, ' ' );
				if ( pos )
					*pos++ = 0;
			}
			//kprint ( "* module %d: [%x - %x] \tcmd = %s\n", i,
			//	  mod->mod_start, mod->mod_end,
			//	  (char *) mod->cmdline );
			kprint ( "* module %d: cmd=%s\n", i,
				  (char *) mod->cmdline );
				  //name );
			//kprint ( "Content:\n%s\n", (char *) mod->mod_start );
			mod++;
		}
	}
#if 0
	if ( mbi->flags & MULTIBOOT_INFO_MEM_MAP )
	{
		kprint ( "Memory map at %x (size=%d)\n",
			  mbi->mmap_addr, mbi->mmap_length );

		multiboot_memory_map_t *memmap = (void *) mbi->mmap_addr;

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

	//kprint ( "\tProcess: at %x, size=%x\n", prog.pi,
	//	  (size_t) prog.pi->end_adr - (size_t) prog.pi->start_adr );

	kprint ( "* Kernel heap:       %x, size=%x",
		  k_heap.start, k_heap.size );
}
