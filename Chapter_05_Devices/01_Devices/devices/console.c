/*! Console as device (wrapper for programs) */

#include "console.h"

int console_send ( void *data, size_t size, uint flags, device_t *dev )
{
	int *p, retval;
	console_t *console;

	p = (int *) data;
	console = dev->params;

	switch ( flags )
	{
		case PRINTCHAR:
			retval = console->print_char ( p[0], p[1] );
			break;

		case CLEAR:
			retval = console->clear ();
			break;

		case GOTOXY:
			retval = console->gotoxy ( p[0], p[1] );
			break;

		default:
			retval = -1;
			break;
	}

	return retval;
}
