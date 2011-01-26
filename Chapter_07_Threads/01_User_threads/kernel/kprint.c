/*! Formated printing on console (using 'console_t' interface) */
#define _KERNEL_

#include "kprint.h"
#include <kernel/devices.h>
#include <lib/string.h>

kdevice_t *k_stdout; /* defined in startup.c */

/*! Print single character to current cursor location with defined color */
static int k_print_char ( int c, int attr )
{
	int p[2];

	p[0] = c;
	p[1] = attr;

	return k_device_send ( p, 2 * sizeof(int), PRINTCHAR, k_stdout );
}

/*! Formated output to console (lightweight version of printf) */
int kprint ( char *format, ... )
{
	char **arg = &format;
	int c;
	char buf[20];

	if ( !format )
		return 0;

	arg++; /* first argument after 'format' (on stack) */

	while ( (c = *format++) != 0 )
	{
		if ( c != '%' )
		{
			k_print_char ( c, KERNEL_FONT );
		}
		else {
			char *p;

			c = *format++;
			switch ( c ) {
			case 'd':
			case 'u':
			case 'x':
			case 'X':
				itoa ( buf, c, *((int *) arg++) );
				p = buf;
				while ( *p )
					k_print_char ( (int) *p++,
						       KERNEL_FONT );
				break;

			case 's':
				p = *arg++;
				if ( !p )
					p = "(null)";

				while ( *p )
					k_print_char ( (int) *p++,
						       KERNEL_FONT );
				break;

			default: /* assuming 'c' */
				k_print_char ( *((int *) arg++), KERNEL_FONT );
				break;
			}
		}
	}

	return 0;
}
