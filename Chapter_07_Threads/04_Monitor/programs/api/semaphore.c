/*! Semaphores (thread synchronization) */

#include "semaphore.h"
#include <api/stdio.h>
#include <api/errno.h>
#include <api/syscalls.h>

int sem_init ( sem_t *sem, int initial_value )
{
	ASSERT_ERRNO_AND_RETURN ( sem, E_INVALID_ARGUMENT );
	return sys__sem_init ( sem, initial_value );
}

int sem_destroy ( sem_t *sem )
{
	ASSERT_ERRNO_AND_RETURN ( sem, E_INVALID_ARGUMENT );
	return sys__sem_destroy ( sem );
}


int sem_post ( sem_t *sem )
{
	ASSERT_ERRNO_AND_RETURN ( sem, E_INVALID_ARGUMENT );
	return sys__sem_post ( sem );
}

int sem_wait ( sem_t *sem )
{
	ASSERT_ERRNO_AND_RETURN ( sem, E_INVALID_ARGUMENT );
	return sys__sem_wait ( sem );
}
