/*! Monitor (synchronization mechanism) */

#pragma once

#include <lib/types.h>

int sys__monitor_init ( monitor_t *monitor );
int sys__monitor_destroy ( monitor_t *monitor );
int sys__monitor_queue_init ( monitor_q *queue );
int sys__monitor_queue_destroy ( monitor_q *queue );

int sys__monitor_lock ( monitor_t *monitor );
int sys__monitor_unlock ( monitor_t *monitor );

int sys__monitor_wait ( monitor_t *monitor, monitor_q *queue );
int sys__monitor_signal ( monitor_q *queue );
int sys__monitor_broadcast ( monitor_q *queue );

#ifdef _KERNEL_

#include <kernel/thread.h>

/* monitor descriptor */
typedef struct _kmonitor_t_
{
	int lock;		/* monitor is owned (locked) or free */

	kthread_t *owner;	/* owner thread descriptor */

	kthread_q queue;	/* queue for blocked threads */
}
kmonitor_t;

/* monitor queue descriptor (conditional variable) */
typedef struct _kmonitor_q_
{
	kthread_q queue;
}
kmonitor_q;

#endif
