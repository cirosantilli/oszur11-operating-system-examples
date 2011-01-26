/*! Time managing functions */
#define _KERNEL_

#define _K_TIME_C_
#include "time.h"

#include <arch/time.h>
#include <arch/interrupts.h>
#include <kernel/thread.h>
#include <kernel/memory.h>
#include <kernel/kprint.h>
#include <kernel/errno.h>
#include <lib/bits.h>
#include <arch/processor.h>


/*! List of active alarms */
static list_t kalarms;

static time_t threshold;

/*!
 * Initialize time management subsystem
 */
void k_time_init ()
{
	/* alarm list is empty */
	list_init ( &kalarms );

	arch_timer_init ();

	arch_get_min_interval ( &threshold );

	threshold.nsec /= 2;
	if ( threshold.sec % 2 )
		threshold.nsec += 1000000000L / 2; /* + half second */
	threshold.sec /= 2;
}

/*!
 * Called from interrupt handler when an alarm has expired
 */
static void k_timer_interrupt ()
{
	if ( k_schedule_alarms () )
		k_schedule_threads ();
}

/*!
 * Iterate through active alarms and activate newly expired ones
 */
static int k_schedule_alarms ()
{
	kalarm_t *first;
	time_t time, ref_time;
	int resched_thr = 0;
	kprocess_t *proc;

	arch_get_time ( &time );
	ref_time = time;
	time_add ( &ref_time, &threshold );

	/* should any alarm be activated? */
	first = list_get ( &kalarms, FIRST );
	while ( first != NULL )
	{
		if ( time_cmp ( &first->alarm.exp_time, &ref_time ) <= 0 )
		{
			/* 'activate' alarm */

			/* but first remove alarm from list */
			first = list_remove ( &kalarms, FIRST, NULL );

			if ( first->alarm.flags & ALARM_PERIODIC )
			{
				/* calculate next activation time */
				time_add ( &first->alarm.exp_time,
					   &first->alarm.period );
				/* put back into list */
				list_sort_add ( &kalarms, first, &first->list,
						alarm_cmp );
			}
			else {
				first->active = 0;
			}

			if ( first->alarm.action )
			{
				/* call directly:

				first->alarm.action ( first->alarm.param );

				   or create new thread for that job: */

				if ( first->thread )
				{ /* alarm scheduled by thread */
				proc = k_get_thread_process ( first->thread );
				k_create_thread (
					first->alarm.action,
					first->alarm.param,
					proc->pi->exit,
					k_get_thread_prio ( first->thread ) + 1,
					NULL, 0, 1,
					proc
				);
				resched_thr++;
				}
				else { /* alarm scheduled by kernel */
				first->alarm.action ( first->alarm.param );
				}
			}

			resched_thr += k_release_all_threads ( &first->queue );

			first = list_get ( &kalarms, FIRST );
		}
		else {
			break;
		}
	}

	first = list_get ( &kalarms, FIRST );
	if ( first )
	{
		ref_time = first->alarm.exp_time;
		time_sub ( &ref_time, &time );
		arch_timer_set ( &ref_time, k_timer_interrupt );
	}

	return resched_thr;
}

/* Add alarm to alarm pool if its expiration time is defined
   If expiration time is in the past, alarm will be immediately activated!
 */
static void k_alarm_add ( kalarm_t *kalarm )
{
	int reschedule = 0;

	/* if exp_time is given (>0) add it into active alarms */
	if ( kalarm->alarm.exp_time.sec + kalarm->alarm.exp_time.nsec > 0 )
	{
		kalarm->active = 1;
		list_sort_add ( &kalarms, kalarm, &kalarm->list, alarm_cmp );
	}
	else {
		kalarm->active = 0;
	}

	reschedule = k_schedule_alarms (); /* this or other alarm may expire */

	SET_ERRNO ( SUCCESS );

	/* block thread? */
	if ( kalarm->active && ( kalarm->alarm.flags & IPC_WAIT ) )
	{
		k_enqueue_thread ( NULL, &kalarm->queue );
		reschedule = 1;
	}

	if ( reschedule )
		k_schedule_threads ();
}


/*! Alarm interface (to other kernel subsystems and threads) ---------------- */

/*!
 * Create new alarm
 * \param alarm Alarm parameters
 * \returns status (0 for success), but also in 'param->alarm.id' is pointer to
 *		newly created alarm (handler)
 */
int k_alarm_new ( void **id, alarm_t *alarm )
{
	kalarm_t *kalarm;

	kalarm = kmalloc ( sizeof (kalarm_t) );
	ASSERT ( kalarm );

	kalarm->alarm = *alarm; /* copy alarm data */
	/* param checking is skipped - assuming all is OK */

	k_threadq_init ( &kalarm->queue );

#ifdef DEBUG
	kalarm->magic = ALARM_MAGIC;
#endif
	*id = kalarm; /* return value = handler */

	kalarm->thread = k_get_active_thread ();

	k_alarm_add ( kalarm );

	RETURN ( SUCCESS );
}

/*!
 * Modify existing alarm (change its values)
 * \param alarm Alarm parameters
 * \returns status (0 for success)
 */
int k_alarm_set ( void *id, alarm_t *alarm )
{
	kalarm_t *kalarm = id;

	ASSERT ( kalarm && kalarm->magic == ALARM_MAGIC );

	kalarm->alarm.action = alarm->action;
	kalarm->alarm.param = alarm->param;
	kalarm->alarm.flags = alarm->flags;
	kalarm->alarm.period = alarm->period;

	SET_ERRNO ( SUCCESS );

	/* is activation time changed? */
	if ( time_cmp ( &kalarm->alarm.exp_time, &alarm->exp_time ) )
	{
		/* remove from active alarms */
		if ( kalarm->active )
			list_remove ( &kalarms, FIRST, &kalarm->list );

		kalarm->alarm.exp_time = alarm->exp_time;

		k_alarm_add ( kalarm );
	}

	RETURN ( SUCCESS );
}

/*!
 * Delete alarm
 * \param param Alarm
 * \returns status (0 for success)
 */
int k_alarm_remove ( void *id )
{
	kalarm_t *kalarm;
	int reschedule = 0;

	kalarm = id;

	ASSERT ( kalarm && kalarm->magic == ALARM_MAGIC );

	/* remove from active alarms (if it was there) */
	if ( kalarm->active )
		list_remove ( &kalarms, FIRST, &kalarm->list );

#ifdef DEBUG
	kalarm->magic = 0;
#endif

	SET_ERRNO ( SUCCESS );

	/* release all waiting threads, if any */
	reschedule = k_release_all_threads ( &kalarm->queue );

	kfree ( kalarm );

	reschedule += k_schedule_alarms ();

	if ( reschedule )
		k_schedule_threads ();

	RETURN ( SUCCESS );
}

/*!
 * Get current time
 * \param time Pointer where to store time
 */
void k_get_time ( time_t *time )
{
	arch_get_time ( time );
}


/*! Interface to threads ---------------------------------------------------- */

/*!
 * Create new alarm
 * \param alarm Alarm parameters
 * \returns status (0 for success), but also in 'param->alarm_id' is pointer to
 *		newly created alarm (handler)
 */
//int sys__alarm_new ( alarm_t *alarm )
int sys__alarm_new ( void *p )
{
	void **id;
	alarm_t *alarm;

	id = *( (void **) p );	p += sizeof ( void *);
	alarm = *( (void **) p );

	ASSERT_ERRNO_AND_EXIT ( id && alarm, E_INVALID_HANDLE );

	id = U2K_GET_ADR ( id, k_get_active_process() );
	alarm = U2K_GET_ADR ( alarm, k_get_active_process() );

	ASSERT_ERRNO_AND_EXIT ( id && alarm, E_INVALID_HANDLE );

	return k_alarm_new ( id, alarm );
}

/*!
 * Modify existing alarm (change its values)
 * \param alarm Alarm parameters
 * \returns status (0 for success)
 */
//int sys__alarm_set ( alarm_t *alarm )
int sys__alarm_set ( void *p )
{
	void *id;
	alarm_t *alarm;

	id = *( (void **) p );	p += sizeof ( void *);
	alarm = *( (void **) p );

	ASSERT_ERRNO_AND_EXIT ( id && alarm, E_INVALID_HANDLE );

	alarm =  U2K_GET_ADR ( alarm, k_get_active_process() );

	return k_alarm_set ( id, alarm );
}

/*!
 * Delete alarm
 * \param alarm Alarm id (pointer)
 * \returns status (0 for success)
 */
int sys__alarm_remove ( void *p )
{
	void *id;

	id = *( (void **) p );

	ASSERT_ERRNO_AND_EXIT ( id, E_INVALID_HANDLE );

	return k_alarm_remove ( id );
}

/*!
 * Retrieve parameters for existing alarm (get its values)
 * \param alarm Pointer where alarm parameters will be stored
 * \returns status (0 for success)
 */
int sys__alarm_get ( void *p )
{
	void *id;
	alarm_t *alarm;
	kalarm_t *kalarm;

	id = *( (void **) p );	p += sizeof ( void *);
	alarm = *( (void **) p );

	ASSERT_ERRNO_AND_EXIT ( id && alarm, E_INVALID_HANDLE );

	alarm =  U2K_GET_ADR ( alarm, k_get_active_process() );

	kalarm = id;

	ASSERT_ERRNO_AND_EXIT ( kalarm && kalarm->magic == ALARM_MAGIC,
				E_INVALID_HANDLE );

	*alarm = kalarm->alarm;

	EXIT ( SUCCESS );
}

/*!
 * Get current time
 * \param time Pointer where to store time
 * \returns status
 */
//int sys__get_time ( time_t *time )
int sys__get_time ( void *p )
{
	time_t *time;

	time = *( (void **) p );

	ASSERT_ERRNO_AND_EXIT ( time, E_INVALID_HANDLE );

	time =  U2K_GET_ADR ( time, k_get_active_process() );


	k_get_time ( time );

	EXIT ( SUCCESS );
}

/*!
 * Wait for alarm to expire (activate)
 * \param alarm Alarm id (pointer)
 * \param wait Do thread really wait or not (just return status)?
 * \returns status (0 for success)
 */
//int sys__wait_for_alarm ( void *alarm, int wait )
int sys__wait_for_alarm ( void *p )
{
	void *id;
	int wait;
	kalarm_t *kalarm;
	int retval;

	id = *( (void **) p );
	ASSERT_ERRNO_AND_EXIT ( id, E_INVALID_HANDLE );


	p += sizeof (void *);
	wait =  *( (int *)   p );

	kalarm = id;

	ASSERT_ERRNO_AND_EXIT ( kalarm && kalarm->magic == ALARM_MAGIC,
				E_INVALID_HANDLE );

	retval = 0;
	SET_ERRNO ( SUCCESS );

	if ( kalarm->active )
	{
		if ( wait & IPC_WAIT )
		{
			k_enqueue_thread ( NULL, &kalarm->queue );
			k_schedule_threads ();
		}
		else {
			SET_ERRNO ( E_NOT_EXPIRED );
			retval = -E_NOT_EXPIRED;
		}
	}

	return retval;
}
