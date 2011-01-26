/*! Time managing functions, wrapper to kernel functions */

#pragma once

#include <lib/types.h>

int time_get ( time_t *t );

int alarm_set ( time_t *expiration, void *func, void *param,
		time_t *period, uint flags );

int alarm_get ( alarm_t *alarm );

int delay ( time_t *t );

int delay_until ( time_t *t );

int alarm_remove ();
