/*! Time managing functions */
#define _KERNEL_

#define _K_TIME_C_
#include "time.h"

#include <arch/time.h>
#include <arch/interrupts.h>
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
	k_schedule_alarms ();
}

/*!
 * Iterate through active alarms and activate newly expired ones
 */
static void k_schedule_alarms ()
{
	kalarm_t *first;
	time_t time, ref_time;

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

			if ( first->alarm.action ) /* activate alarm */
				first->alarm.action ( first->alarm.param );

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
}

/* Add alarm to alarm pool if its expiration time is defined
   If expiration time is in the past, alarm will be immediately activated!
 */
static void k_alarm_add ( kalarm_t *kalarm )
{
	/* if exp_time is given (>0) add it into active alarms */
	if ( kalarm->alarm.exp_time.sec + kalarm->alarm.exp_time.nsec > 0 )
	{
		kalarm->active = 1;
		list_sort_add ( &kalarms, kalarm, &kalarm->list, alarm_cmp );
	}
	else {
		kalarm->active = 0;
	}

	k_schedule_alarms (); /* this or other alarm may expire */
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

#ifdef DEBUG
	kalarm->magic = ALARM_MAGIC;
#endif
	*id = kalarm; /* return value = handler */

	k_alarm_add ( kalarm );

	return SUCCESS;
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

	/* is activation time changed? */
	if ( time_cmp ( &kalarm->alarm.exp_time, &alarm->exp_time ) )
	{
		/* remove from active alarms */
		if ( kalarm->active )
			list_remove ( &kalarms, FIRST, &kalarm->list );

		kalarm->alarm.exp_time = alarm->exp_time;

		k_alarm_add ( kalarm );
	}

	return SUCCESS;
}

/*!
 * Delete alarm
 * \param param Alarm
 * \returns status (0 for success)
 */
int k_alarm_remove ( void *id )
{
	kalarm_t *kalarm;

	kalarm = id;

	ASSERT ( kalarm && kalarm->magic == ALARM_MAGIC );

	/* remove from active alarms (if it was there) */
	if ( kalarm->active )
		list_remove ( &kalarms, FIRST, &kalarm->list );

#ifdef DEBUG
	kalarm->magic = 0;
#endif

	kfree ( kalarm );

	k_schedule_alarms ();

	return SUCCESS;
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
int sys__alarm_new ( void **id, alarm_t *alarm )
{
	int retval;

	ASSERT_ERRNO_AND_EXIT ( id && alarm, E_INVALID_HANDLE );

	disable_interrupts ();
	retval = k_alarm_new ( id, alarm );
	enable_interrupts ();

	return retval;
}

/*!
 * Modify existing alarm (change its values)
 * \param alarm Alarm parameters
 * \returns status (0 for success)
 */
int sys__alarm_set ( void *id, alarm_t *alarm )
 {
	int retval;

	ASSERT_ERRNO_AND_EXIT ( id && alarm, E_INVALID_HANDLE );

	disable_interrupts ();
	retval = k_alarm_set ( id, alarm );
	enable_interrupts ();

	return retval;
}

/*!
 * Delete alarm
 * \param alarm Alarm id (pointer)
 * \returns status (0 for success)
 */
int sys__alarm_remove ( void *id )
{
	int retval;

	ASSERT_ERRNO_AND_EXIT ( id, E_INVALID_HANDLE );

	disable_interrupts ();
	retval = k_alarm_remove ( id );
	enable_interrupts ();

	return retval;
}

/*!
 * Retrieve parameters for existing alarm (get its values)
 * \param alarm Pointer where alarm parameters will be stored
 * \returns status (0 for success)
 */
int sys__alarm_get ( void *id, alarm_t *alarm )
{
	kalarm_t *kalarm;

	ASSERT_ERRNO_AND_EXIT ( id && alarm, E_INVALID_HANDLE );

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
int sys__get_time ( time_t *time )
{
	ASSERT_ERRNO_AND_EXIT ( time, E_INVALID_HANDLE );

	disable_interrupts ();
	k_get_time ( time );
	enable_interrupts ();

	EXIT ( SUCCESS );
}

