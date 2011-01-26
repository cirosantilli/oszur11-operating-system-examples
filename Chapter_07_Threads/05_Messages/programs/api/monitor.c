/*! Thread management functions */

#include "monitor.h"
#include <api/stdio.h>
#include <api/errno.h>
#include <api/syscalls.h>

int monitor_init ( monitor_t *monitor )
{
	ASSERT_ERRNO_AND_RETURN ( monitor, E_INVALID_ARGUMENT );
	return sys__monitor_init ( monitor );
}

int monitor_destroy ( monitor_t *monitor )
{
	ASSERT_ERRNO_AND_RETURN ( monitor, E_INVALID_ARGUMENT );
	return sys__monitor_destroy ( monitor );
}

int monitor_queue_init ( monitor_q *queue )
{
	ASSERT_ERRNO_AND_RETURN ( queue, E_INVALID_ARGUMENT );
	return sys__monitor_queue_init ( queue );
}

int monitor_queue_destroy ( monitor_q *queue )
{
	ASSERT_ERRNO_AND_RETURN ( queue, E_INVALID_ARGUMENT );
	return sys__monitor_queue_destroy ( queue );
}

int monitor_lock ( monitor_t *monitor )
{
	ASSERT_ERRNO_AND_RETURN ( monitor, E_INVALID_ARGUMENT );
	return sys__monitor_lock ( monitor );
}

int monitor_unlock ( monitor_t *monitor )
{
	ASSERT_ERRNO_AND_RETURN ( monitor, E_INVALID_ARGUMENT );
	return sys__monitor_unlock ( monitor );
}

int monitor_wait ( monitor_t *monitor, monitor_q *queue )
{
	ASSERT_ERRNO_AND_RETURN ( monitor && queue, E_INVALID_ARGUMENT );
	return sys__monitor_wait ( monitor, queue );
}

int monitor_signal ( monitor_q *queue )
{
	ASSERT_ERRNO_AND_RETURN ( queue, E_INVALID_ARGUMENT );
	return sys__monitor_signal ( queue );
}

int monitor_broadcast ( monitor_q *queue )
{
	ASSERT_ERRNO_AND_RETURN ( queue, E_INVALID_ARGUMENT );
	return sys__monitor_broadcast ( queue );
}
