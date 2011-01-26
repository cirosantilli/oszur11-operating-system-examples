/*! Time managing functions */

#include "time.h"
#include <lib/types.h>
#include <api/stdio.h>
#include <api/errno.h>
#include <api/syscalls.h>

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
 * \param id Pointer to existing alarm, or NULL for new
 * \param expiration When alarm should be activated
 * \param func Function that should be invoked upon alarm expiration (activation)
 * \param param Parameter which will be passed to 'func'
 * \param period Period for repeating calls to 'func' (alarm is periodic if this
 *               parameter is set)
 * \returns Pointer to newly created alarm
 */
void *alarm_set ( void *id, time_t *expiration, void *func, void *param,
		  time_t *period, uint flags )
{
	alarm_t alarm;
	int ret_val;

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

	if ( !id )
		ret_val = sys__alarm_new ( &id, &alarm );
	else
		ret_val = sys__alarm_set ( id, &alarm );

	if ( !ret_val )
		return id;
	else
		return NULL;
}

/*!
 * Get alarm parameters
 * \param id Pointer to alarm handler
 * \param alarm Pointer where to store parameters
 * \returns 0 if successful, -1 otherwise
 */
int alarm_get ( void *id, alarm_t *alarm )
{
	ASSERT_ERRNO_AND_RETURN ( id && alarm, E_INVALID_ARGUMENT );
	return sys__alarm_get ( id, alarm );
}

/*!
 * Delay thread until given time is reached (passes)
 * Given time is compared by system time
 * \param t Expiration time
 * \returns 0
 */
int delay_until ( time_t *t )
{
	void *id;

	ASSERT_ERRNO_AND_RETURN ( t, E_INVALID_ARGUMENT );

	id = alarm_set ( NULL, t, NULL, NULL, NULL, IPC_WAIT
 );

	alarm_remove ( id );

	return 0;
}

/*!
 * Delay thread until given time period expires
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
 * \param id Pointer to alarm handle
 * \returns 0 if successful, -1 otherwise
 */
int alarm_remove ( void *id )
{
	ASSERT_ERRNO_AND_RETURN ( id, E_INVALID_ARGUMENT );
	return sys__alarm_remove ( id );
}

/*!
 * Wait for alarm to expire (activate)
 * \param alarm Alarm id (pointer)
 * \param wait Do thread really wait or not (just return status)?
 * \returns status (0 for success)
 */
int wait_for_alarm ( void *id, int wait )
{
	ASSERT_ERRNO_AND_RETURN ( id, E_INVALID_ARGUMENT );
	return sys__wait_for_alarm ( id, wait );
}
