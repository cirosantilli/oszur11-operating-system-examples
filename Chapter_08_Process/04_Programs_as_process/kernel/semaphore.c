/*! Simple semaphore */
#define _KERNEL_

#define _SEMAPHORE_C_
#include "semaphore.h"

#include <kernel/thread.h>
#include <kernel/memory.h>
#include <kernel/kprint.h>
#include <kernel/errno.h>
#include <lib/types.h>

/*!
 * Initialize new semaphore with initial value
 */
int sys__sem_init ( void *p )
{
	sem_t *sem;
	int initial_value;
	ksem_t *ksem;

	sem = U2K_GET_ADR ( *( (void **) p ), k_get_active_process() );
	p += sizeof (void *);
	initial_value = *( (int *) p );

	ASSERT_ERRNO_AND_EXIT ( sem, E_INVALID_HANDLE );

	ksem = kmalloc ( sizeof (ksem_t) );
	ASSERT ( ksem );

	ksem->sem_value = initial_value;
	k_threadq_init ( &ksem->queue );

	sem->ptr = ksem;

	EXIT ( SUCCESS );
}

/*!
 * Destroy semaphore (and unblock all threads blocked on it)
 */
int sys__sem_destroy ( void *p )
{
	ksem_t *ksem;
	sem_t *sem;

	sem = U2K_GET_ADR ( *( (void **) p ), k_get_active_process() );

	ASSERT_ERRNO_AND_EXIT ( sem && sem->ptr, E_INVALID_HANDLE );

	ksem = sem->ptr;

	if ( k_release_all_threads ( &ksem->queue ) )
		k_schedule_threads ();

	kfree ( ksem );
	sem->ptr = NULL;

	EXIT ( SUCCESS );
}

/*!
 * Increment semaphore value by 1 or unblock single blocked thread on it
 */
int sys__sem_post ( void *p )
{
	ksem_t *ksem;
	sem_t *sem;

	sem = U2K_GET_ADR ( *( (void **) p ), k_get_active_process() );

	ASSERT_ERRNO_AND_EXIT ( sem && sem->ptr, E_INVALID_HANDLE );

	ksem = sem->ptr;

	SET_ERRNO ( SUCCESS );

	if ( !k_release_thread ( &ksem->queue ) )
		ksem->sem_value++;
	else
		k_schedule_threads ();

	RETURN ( SUCCESS );
}

/*!
 * Decrement semaphore value by 1 or block calling thread if value == 0
 */
int sys__sem_wait ( void *p )
{
	ksem_t *ksem;
	sem_t *sem;

	sem = U2K_GET_ADR ( *( (void **) p ), k_get_active_process() );

	ASSERT_ERRNO_AND_EXIT ( sem && sem->ptr, E_INVALID_HANDLE );

	ksem = sem->ptr;

	SET_ERRNO ( SUCCESS );

	if ( ksem->sem_value > 0 )
	{
		ksem->sem_value--;
	}
	else {
		k_enqueue_thread ( NULL, &ksem->queue );
		k_schedule_threads ();
	}

	RETURN ( SUCCESS );
}
