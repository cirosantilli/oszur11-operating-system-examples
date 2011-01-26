/*! Timer in arch layer; use 'arch_timer_t' device defined in configuration */

#include "time.h"

#include <lib/types.h>

extern arch_timer_t TIMER;
static arch_timer_t *timer = &TIMER;

static time_t clock;	/* system time starting from 0:00 at power on */
static time_t delay;	/* delay set by kernel, or timer->max_count */
static time_t last_load;/* last time equivalent loaded to counter */

static time_t threshold;/* timer->min_interval / 2 */

static void (*alarm_handler) (); /* kernel function - call when alarm given by
				    kernel ('delay') expires */

static void arch_timer_handler (); /* whenever timer expires call this */

void arch_enable_timer_interrupt ()	{ timer->enable_interrupt ();	}
void arch_disable_timer_interrupt ()	{ timer->disable_interrupt ();	}

void arch_get_min_interval ( time_t *time ) { *time = timer->min_interval; }

/*!
 * Initialize timer 'arch' subsystem: timer device, subsystem data
 */
void arch_timer_init ()
{
	clock.sec = clock.nsec = 0;

	timer->init ();

	last_load = delay = timer->max_interval;

	timer->set_interval ( &last_load );
	timer->register_interrupt ( arch_timer_handler );
	timer->enable_interrupt ();

	threshold.sec = timer->min_interval.sec / 2;
	threshold.nsec = timer->min_interval.nsec / 2;
	if ( timer->min_interval.sec % 2 )
		threshold.nsec += 1000000000L / 2; /* + half second */

	return;
}

/*!
 * Set next timer activation
 * \param time Time of next activation
 * \param alarm_func Function to call upon timer expiration
 */
void arch_timer_set ( time_t *time, void *alarm_func )
{
	time_t remainder;

	timer->get_interval_remainder ( &remainder );
	time_sub ( &last_load, &remainder );
	time_add ( &clock, &last_load );

	delay = *time;
	if ( time_cmp ( &delay, &timer->min_interval ) < 0 )
		delay = timer->min_interval;

	alarm_handler = alarm_func;

	if ( time_cmp ( &delay, &timer->max_interval ) > 0 )
		last_load = timer->max_interval;
	else
		last_load = delay;

	timer->set_interval ( &last_load );
}

/*!
 * Get 'current' system time
 * \param time Store address for current time
 */
void arch_get_time ( time_t *time )
{
	time_t remainder;

	timer->get_interval_remainder ( &remainder );

	*time = last_load;
	time_sub ( time, &remainder );
	time_add ( time, &clock );
}

/*!
 * Registered 'arch' handler for timer interrupts;
 * update system time and forward interrupt to kernel if its timer is expired
 */
static void arch_timer_handler ()
{
	void (*k_handler) ();

	time_add ( &clock, &last_load );

	time_sub ( &delay, &last_load );
	last_load = timer->max_interval;

	if ( time_cmp ( &delay, &threshold ) <= 0 )
	{
		delay = timer->max_interval;
		timer->set_interval ( &last_load );

		k_handler = alarm_handler;
		alarm_handler = NULL; /* reset kernel callback function */

		if ( k_handler )
			k_handler (); /* forward interrupt to kernel */
	}
	else {
		if ( time_cmp ( &delay, &last_load ) < 0 )
			last_load = delay;

		timer->set_interval ( &last_load );
	}
}
