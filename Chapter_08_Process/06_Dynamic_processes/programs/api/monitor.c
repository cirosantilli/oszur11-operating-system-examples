/*! Thread management functions */

#include "monitor.h"
#include <api/syscall.h>
#include <api/stdio.h>
#include <api/errno.h>

int monitor_init ( monitor_t *monitor )
{
	ASSERT_ERRNO_AND_RETURN ( monitor, E_INVALID_ARGUMENT );
	return syscall ( MONITOR_INIT, monitor );
}

int monitor_destroy ( monitor_t *monitor )
{
	ASSERT_ERRNO_AND_RETURN ( monitor, E_INVALID_ARGUMENT );
	return syscall ( MONITOR_DESTROY, monitor );
}

int monitor_queue_init ( monitor_q *queue )
{
	ASSERT_ERRNO_AND_RETURN ( queue, E_INVALID_ARGUMENT );
	return syscall ( MONITOR_QUEUE_INIT, queue );
}

int monitor_queue_destroy ( monitor_q *queue )
{
	ASSERT_ERRNO_AND_RETURN ( queue, E_INVALID_ARGUMENT );
	return syscall ( MONITOR_QUEUE_DESTROY, queue );
}

int monitor_lock ( monitor_t *monitor )
{
	ASSERT_ERRNO_AND_RETURN ( monitor, E_INVALID_ARGUMENT );
	return syscall ( MONITOR_LOCK, monitor );
}

int monitor_unlock ( monitor_t *monitor )
{
	ASSERT_ERRNO_AND_RETURN ( monitor, E_INVALID_ARGUMENT );
	return syscall ( MONITOR_UNLOCK, monitor );
}

int monitor_wait ( monitor_t *monitor, monitor_q *queue )
{
	ASSERT_ERRNO_AND_RETURN ( monitor && queue, E_INVALID_ARGUMENT );
	return syscall ( MONITOR_WAIT, monitor, queue );
}

int monitor_signal ( monitor_q *queue )
{
	ASSERT_ERRNO_AND_RETURN ( queue, E_INVALID_ARGUMENT );
	return syscall ( MONITOR_SIGNAL, queue );
}

int monitor_broadcast ( monitor_q *queue )
{
	ASSERT_ERRNO_AND_RETURN ( queue, E_INVALID_ARGUMENT );
	return syscall ( MONITOR_BROADCAST, queue );
}
