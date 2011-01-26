/*! Monitor (synchronization mechanism) */
#define _KERNEL_

#define _MONITOR_C_
#include "monitor.h"

#include <kernel/thread.h>
#include <kernel/memory.h>
#include <kernel/kprint.h>
#include <kernel/errno.h>
#include <lib/types.h>

/*!
 * Initialize new monitor
 */
int sys__monitor_init ( monitor_t *monitor )
{
	kmonitor_t *kmonitor;

	ASSERT_ERRNO_AND_EXIT ( monitor, E_INVALID_HANDLE );

	disable_interrupts ();

	kmonitor = kmalloc ( sizeof (kmonitor_t) );
	ASSERT ( kmonitor );

	kmonitor->lock = FALSE;
	kmonitor->owner = NULL;
	k_threadq_init ( &kmonitor->queue );

	monitor->ptr = kmonitor;

	enable_interrupts ();

	EXIT ( SUCCESS );
}

/*!
 * Destroy monitor (and unblock all threads blocked on it)
 */
int sys__monitor_destroy ( monitor_t *monitor )
{
	kmonitor_t *kmonitor;

	ASSERT_ERRNO_AND_EXIT ( monitor && monitor->ptr, E_INVALID_HANDLE );

	disable_interrupts ();

	kmonitor = monitor->ptr;

	if ( k_release_all_threads ( &kmonitor->queue ) )
		k_schedule_threads ();

	kfree ( kmonitor );
	monitor->ptr = NULL;

	enable_interrupts ();

	EXIT ( SUCCESS );
}

/*!
 * Initialize new monitor
 */
int sys__monitor_queue_init ( monitor_q *queue )
{
	kmonitor_q *kqueue;

	ASSERT_ERRNO_AND_EXIT ( queue, E_INVALID_HANDLE );

	disable_interrupts ();

	kqueue = kmalloc ( sizeof (kmonitor_q) );
	ASSERT ( kqueue );

	k_threadq_init ( &kqueue->queue );

	queue->ptr = kqueue;

	enable_interrupts ();

	EXIT ( SUCCESS );
}

/*!
 * Destroy monitor (and unblock all threads blocked on it)
 */
int sys__monitor_queue_destroy ( monitor_q *queue )
{
	kmonitor_q *kqueue;

	ASSERT_ERRNO_AND_EXIT ( queue && queue->ptr, E_INVALID_HANDLE );

	disable_interrupts ();

	kqueue = queue->ptr;

	if ( k_release_all_threads ( &kqueue->queue ) )
		k_schedule_threads ();

	kfree ( kqueue );
	queue->ptr = NULL;

	enable_interrupts ();

	EXIT ( SUCCESS );
}

/*!
 * Lock monitor (or block trying)
 */
int sys__monitor_lock ( monitor_t *monitor )
{
	kmonitor_t *kmonitor;

	ASSERT_ERRNO_AND_EXIT ( monitor && monitor->ptr, E_INVALID_HANDLE );

	disable_interrupts ();

	kmonitor = monitor->ptr;

	SET_ERRNO ( SUCCESS );

	if ( !kmonitor->lock )
	{
		kmonitor->lock = TRUE;
		kmonitor->owner = k_get_active_thread ();
	}
	else {
		k_enqueue_thread ( NULL, &kmonitor->queue );
		k_schedule_threads ();
	}

	enable_interrupts ();

	EXIT ( SUCCESS );
}

/*!
 * Unlock monitor (or block trying)
 */
int sys__monitor_unlock ( monitor_t *monitor )
{
	kmonitor_t *kmonitor;

	ASSERT_ERRNO_AND_EXIT ( monitor && monitor->ptr, E_INVALID_HANDLE );

	kmonitor = monitor->ptr;

	ASSERT_ERRNO_AND_EXIT ( kmonitor->owner == k_get_active_thread (),
				E_NOT_OWNER );

	disable_interrupts ();

	SET_ERRNO ( SUCCESS );

	kmonitor->owner = k_threadq_get ( &kmonitor->queue );
	if ( !k_release_thread ( &kmonitor->queue ) )
		kmonitor->lock = FALSE;
	else
		k_schedule_threads ();

	enable_interrupts ();

	EXIT ( SUCCESS );
}

/*!
 * Block thread (on conditional variable) and release monitor
 */
int sys__monitor_wait ( monitor_t *monitor, monitor_q *queue )
{
	kmonitor_t *kmonitor;
	kmonitor_q *kqueue;

	ASSERT_ERRNO_AND_EXIT ( monitor && monitor->ptr, E_INVALID_HANDLE );
	ASSERT_ERRNO_AND_EXIT ( queue && queue->ptr, E_INVALID_HANDLE );

	kmonitor = monitor->ptr;
	kqueue = queue->ptr;

	ASSERT_ERRNO_AND_EXIT ( kmonitor->owner == k_get_active_thread (),
				E_NOT_OWNER );

	disable_interrupts ();

	SET_ERRNO ( SUCCESS );

	k_set_thread_qdata ( NULL, kmonitor );
	k_enqueue_thread ( NULL, &kqueue->queue );

	kmonitor->owner = k_threadq_get ( &kmonitor->queue );
	if ( !k_release_thread ( &kmonitor->queue ) )
		kmonitor->lock = FALSE;

	k_schedule_threads ();

	enable_interrupts ();

	EXIT ( SUCCESS );
}

/* 'signal' and 'broadcast' are very similar - implemented in single function */
static int k_monitor_release ( monitor_q *queue, int broadcast );

/*!
 * Unblock thread from monitor queue
 */
int sys__monitor_signal ( monitor_q *queue )
{
	return k_monitor_release ( queue, FALSE );
}

/*!
 * Unblock all threads from monitor queue
 */
int sys__monitor_broadcast ( monitor_q *queue )
{
	return k_monitor_release ( queue, TRUE );
}

/*! Release first or all threads from monitor queue (cond.var.) */
static int k_monitor_release ( monitor_q *queue, int broadcast )
{
	kmonitor_q *kqueue;
	kthread_t *kthr;
	kmonitor_t *kmonitor;
	int reschedule = 0;

	ASSERT_ERRNO_AND_EXIT ( queue && queue->ptr, E_INVALID_HANDLE );

	disable_interrupts ();

	kqueue = queue->ptr;

	do {
		kthr = k_threadq_get ( &kqueue->queue ); /* first from queue */

		if ( !kthr )
			break;

		kmonitor = k_get_thread_qdata ( kthr );

		if ( !kmonitor->lock ) /* monitor not locked? */
		{
			/* unblocked thread becomes monitor owner */
			kmonitor->lock = TRUE;
			kmonitor->owner = kthr;
			k_release_thread ( &kqueue->queue );/*to ready threads*/
			reschedule++;
		}
		else {
			/* move thread from monitor queue (cond.var.)
			   to monitor entrance queue */
			kthr = k_threadq_remove ( &kqueue->queue, NULL );
			k_enqueue_thread ( kthr, &kmonitor->queue );
		}
	}
	while ( kthr && broadcast );

	SET_ERRNO ( SUCCESS );

	if ( reschedule )
		k_schedule_threads ();

	enable_interrupts ();

	EXIT ( SUCCESS );
}
