/*! Timer in arch layer; use 'arch_timer_t' device defined in configuration */

#pragma once

#include <lib/types.h>

/* define timer device interface */
typedef struct _arch_timer_t_
{
	time_t min_interval;
	time_t max_interval;

	void (*init) ();
	void (*set_interval) ( time_t * );
	void (*get_interval_remainder) ( time_t * );
	void (*enable_interrupt) ();
	void (*disable_interrupt) ();
	void (*register_interrupt) ( void *handler );
}
arch_timer_t;

/* kernel interface */
void arch_timer_init ();
void arch_timer_set ( time_t *time, void *alarm_func );
void arch_get_time ( time_t *time );
void arch_get_min_interval ( time_t *time );

void arch_enable_timer_interrupt ();
void arch_disable_timer_interrupt ();
