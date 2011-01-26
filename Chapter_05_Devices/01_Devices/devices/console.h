/*! Console interface */

#pragma once

#include <devices/devices.h>
#include <lib/list.h>
#include <lib/types.h>

/*! Console interface (used in kernel mode) */
typedef struct _console_t_
{
	int (*init) (void *);
	int (*clear) ();
	int (*gotoxy) ( int x, int y );
	int (*print_char) ( int c, int attr );
}
console_t;

/*! device wrapper */
int console_send ( void *data, size_t size, uint flags, device_t *dev );
