/*! Monitor (synchronization mechanism) */

#pragma once

#include <lib/types.h>
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

int sys__monitor_init ( void *p );
int sys__monitor_destroy ( void *p );
int sys__monitor_queue_init ( void *p );
int sys__monitor_queue_destroy ( void *p );

int sys__monitor_lock ( void *p );
int sys__monitor_unlock ( void *p );
int sys__monitor_wait ( void *p );
int sys__monitor_signal ( void *p );
int sys__monitor_broadcast ( void *p );

