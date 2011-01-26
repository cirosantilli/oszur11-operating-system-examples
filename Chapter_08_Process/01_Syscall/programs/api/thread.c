/*! Threads */

#include "thread.h"

#include <api/stdio.h>
#include <api/syscall.h>
#include <api/errno.h>
#include <lib/types.h>

/*! Thread creation/exit/wait/cancel ---------------------------------------- */

int create_thread ( void *start_func, void *param, int prio, thread_t *handle )
{
	ASSERT_ERRNO_AND_RETURN ( start_func, E_INVALID_ARGUMENT );
	return syscall ( CREATE_THREAD, start_func, param, prio, handle );
}

void thread_exit ( int status )
{
	syscall ( THREAD_EXIT, status );
}

int wait_for_thread ( void *thread, int wait )
{
	int retval;

	ASSERT_ERRNO_AND_RETURN ( thread, E_INVALID_ARGUMENT );

	do {
		retval = syscall ( WAIT_FOR_THREAD, thread, wait );
	}
	while ( retval == -E_RETRY && wait );

	return retval;
}

int cancel_thread ( void *thread )
{
	ASSERT_ERRNO_AND_RETURN ( thread, E_INVALID_ARGUMENT );
	return syscall ( CANCEL_THREAD, thread );
}

int thread_self ( thread_t *thread )
{
	ASSERT_ERRNO_AND_RETURN ( thread, E_INVALID_ARGUMENT );
	return syscall ( THREAD_SELF, thread );
}
