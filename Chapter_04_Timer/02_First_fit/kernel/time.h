/*! Time managing functions */

#pragma once

#include <lib/types.h>

/*! interface to programs */
int sys__get_time ( time_t *time );
int sys__alarm_set ( alarm_t *alarm );
int sys__alarm_get ( alarm_t *alarm );
int sys__alarm_remove ();

#ifdef _KERNEL_

/*! interface to kernel */
void k_time_init ();
void k_get_time ( time_t *time );

#ifdef	_K_TIME_C_

/*! local functions */
static void k_timer_interrupt ();
static void k_set_alarm ();

#endif	/* _K_TIME_C_ */
#endif /* _KERNEL_ */
