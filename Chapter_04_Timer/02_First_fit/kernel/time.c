/*! Time managing functions */
#define _KERNEL_

#define _K_TIME_C_
#include "time.h"

#include <arch/time.h>
#include <arch/interrupts.h>
#include <kernel/kprint.h>
#include <kernel/errno.h>
#include <lib/bits.h>
#include <arch/processor.h>

static alarm_t kalarm;

/*!
 * Initialize time management subsystem
 */
void k_time_init ()
{
	arch_timer_init ();

	sys__alarm_remove (); /* reset */
}

static void k_set_alarm ()
{
	time_t t1, t0;

	t1 = kalarm.exp_time;
	k_get_time ( &t0 );
	time_sub ( &t1, &t0 );

	arch_timer_set ( &t1, k_timer_interrupt );
}

/*!
 * Called from interrupt handler (arch layer) when alarm has expired
 */
static void k_timer_interrupt ()
{
	if ( kalarm.exp_time.sec + kalarm.exp_time.nsec > 0 )
	{
		if ( kalarm.action ) /* activate alarm */
			kalarm.action ( kalarm.param );

		if ( kalarm.flags & ALARM_PERIODIC )
		{
			/* calculate next activation time */
			time_add ( &kalarm.exp_time, &kalarm.period );

			k_set_alarm ();
		}
	}
}

/*!
 * Get current time
 * \param time Pointer where to store time
 */
void k_get_time ( time_t *time )
{
	arch_get_time ( time );
}


/*! Interface to programs --------------------------------------------------- */

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

/*!
 * Define alarm (change its values)
 * \param alarm Alarm parameters
 * \returns status (0 for success)
 */
int sys__alarm_set ( alarm_t *alarm )
 {
	ASSERT_ERRNO_AND_EXIT ( alarm, E_INVALID_HANDLE );

	disable_interrupts ();

	kalarm = *alarm;

	k_set_alarm ();

	enable_interrupts ();

	return 0;
}

/*!
 * Retrieve alarm (its current values)
 * \param alarm Alarm parameters
 * \returns status (0 for success)
 */
int sys__alarm_get ( alarm_t *alarm )
 {
	ASSERT_ERRNO_AND_EXIT ( alarm, E_INVALID_HANDLE );

	disable_interrupts ();

	*alarm = kalarm;

	enable_interrupts ();

	return 0;
}

/*!
 * Delete alarm
 * \returns status (0 for success)
 */
int sys__alarm_remove ()
{
	kalarm.exp_time.sec = kalarm.exp_time.nsec = 0;
	kalarm.period.sec = kalarm.period.nsec = 0;
	kalarm.action = NULL;
	kalarm.param = NULL;
	kalarm.flags = 0;

	return 0;
}
