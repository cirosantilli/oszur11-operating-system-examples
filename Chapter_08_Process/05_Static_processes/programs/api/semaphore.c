/*! Semaphores (thread synchronization) */

#include "semaphore.h"
#include <api/syscall.h>
#include <api/stdio.h>
#include <api/errno.h>

int sem_init ( sem_t *sem, int initial_value )
{
	ASSERT_ERRNO_AND_RETURN ( sem, E_INVALID_ARGUMENT );
	return syscall ( SEM_INIT, sem, initial_value );
}

int sem_destroy ( sem_t *sem )
{
	ASSERT_ERRNO_AND_RETURN ( sem, E_INVALID_ARGUMENT );
	return syscall ( SEM_DESTROY, sem );
}


int sem_post ( sem_t *sem )
{
	ASSERT_ERRNO_AND_RETURN ( sem, E_INVALID_ARGUMENT );
	return syscall ( SEM_POST, sem );
}

int sem_wait ( sem_t *sem )
{
	ASSERT_ERRNO_AND_RETURN ( sem, E_INVALID_ARGUMENT );
	return syscall ( SEM_WAIT, sem );
}
