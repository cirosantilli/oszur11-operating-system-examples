/*! Threads */

#include "thread.h"

#include <api/stdio.h>
#include <api/malloc.h>
#include <api/errno.h>
#include <lib/types.h>
#include <api/syscalls.h>

/*! Thread creation/exit/wait/cancel ---------------------------------------- */

int create_thread ( void *start_func, void *param, int prio, thread_t *handle )
{
	ASSERT_ERRNO_AND_RETURN ( start_func, E_INVALID_ARGUMENT );
	return sys__create_thread ( start_func, param, prio, handle );
}

void thread_exit ( int status )
{
	sys__thread_exit ( status );
}

int wait_for_thread ( void *thread, int wait )
{
	int retval;

	ASSERT_ERRNO_AND_RETURN ( thread, E_INVALID_ARGUMENT );

	do {
		retval = sys__wait_for_thread ( thread, wait );
	}
	while ( retval == -E_RETRY && wait );

	return retval;
}

int cancel_thread ( void *thread )
{
	ASSERT_ERRNO_AND_RETURN ( thread, E_INVALID_ARGUMENT );
	return sys__cancel_thread ( thread );
}

int thread_self ( thread_t *thread )
{
	ASSERT_ERRNO_AND_RETURN ( thread, E_INVALID_ARGUMENT );
	return sys__thread_self ( thread );
}
