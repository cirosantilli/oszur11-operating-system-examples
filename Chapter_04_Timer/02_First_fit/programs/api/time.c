/*! Time managing functions */

#include "time.h"
#include <lib/types.h>
#include <api/stdio.h>
#include <api/errno.h>
#include <api/syscalls.h>
#include <arch/processor.h>

/*!
 * Get current system time
 * \param t Pointer where to store time
 * \returns 0 if successful, -1 otherwise
 */
int time_get ( time_t *t )
{
	ASSERT_ERRNO_AND_RETURN ( t, E_INVALID_ARGUMENT );
	return sys__get_time ( t );
}

/*!
 * Set new or change existing alarm
 * \param expiration When alarm should be activated
 * \param func Function that should be invoked upon alarm expiration (activation)
 * \param param Parameter which will be passed to 'func'
 * \param period Period for repeating calls to 'func' (alarm is periodic if this
 *               parameter is set)
 * \returns 0 if succesfull
 */
int alarm_set ( time_t *expiration, void *func, void *param,
		time_t *period, uint flags )
{
	alarm_t alarm;

	alarm.flags = flags;

	if ( expiration && expiration->sec + expiration->nsec > 0 )
		alarm.exp_time = *expiration;
	else
		alarm.exp_time.sec = alarm.exp_time.nsec = 0;

	alarm.action = func;
	alarm.param = param;

	if ( period && period->sec + period->nsec > 0 )
	{
		alarm.period = *period;
		alarm.flags |= ALARM_PERIODIC;
	}
	else {
		alarm.period.sec = alarm.period.nsec = 0;
	}

	return sys__alarm_set ( &alarm );
}

/*!
 * Get alarm parameters
 * \param alarm Pointer where to store parameters
 * \returns 0 if successful, -1 otherwise
 */
int alarm_get ( alarm_t *alarm )
{
	ASSERT_ERRNO_AND_RETURN ( alarm, E_INVALID_ARGUMENT );
	return sys__alarm_get ( alarm );
}

/* mark timer expired for waiting program */
static void alarm_expired ( void *p )
{
	*( (int *) p ) = TRUE;
}

/*!
 * Delay program until given time is reached (passes)
 * Given time is compared by system time
 * \param t Expiration time
 * \returns 0
 */
int delay_until ( time_t *t )
{
	volatile int elapsed = 0;

	ASSERT_ERRNO_AND_RETURN ( t, E_INVALID_ARGUMENT );

	alarm_set ( t, alarm_expired, (void *) &elapsed, NULL, 0 );

	/* wait until alarm expires */
	while ( !elapsed )
		suspend ();

	return 0;
}

/*!
 * Delay program until given time period expires
 * \param t Expiration time
 * \returns 0
 */
int delay ( time_t *t )
{
	time_t aps;

	ASSERT_ERRNO_AND_RETURN ( t, E_INVALID_ARGUMENT );
	time_get ( &aps );

	aps.sec += t->sec;
	aps.nsec += t->nsec;
	if ( aps.nsec > 1000000000 )
	{
		aps.nsec -= 1000000000;
		aps.sec++;
	}

	return delay_until ( &aps );
}

/*!
 * Removes (deletes) alarm
 * \returns 0 if successful, -1 otherwise
 */
int alarm_remove ()
{
	return sys__alarm_remove ();
}
