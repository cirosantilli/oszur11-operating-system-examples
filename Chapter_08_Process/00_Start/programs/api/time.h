/*! Time managing functions, wrapper to kernel functions */

#pragma once

#include <lib/types.h>

void *alarm_set ( void *id, time_t *expiration, void *func, void *param,
		  time_t *period, uint flags );

int alarm_get ( void *id, alarm_t *alarm );

int delay ( time_t *t );

int delay_until ( time_t *t );

int time_get ( time_t *t );

int alarm_remove ( void *id );

int wait_for_alarm ( void *id, int wait );
