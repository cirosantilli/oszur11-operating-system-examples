/*! Formated printing on console (using 'console_t' interface) */
#define _KERNEL_

#include "kprint.h"
#include <devices/console.h>
#include <lib/string.h>

extern console_t *k_stdout; /* defined in startup.c */

/*! Formated output to console (lightweight version of printf) */
int kprint ( char *format, ... )
{
	char **arg = &format;
	int c;
	char buf[20];

	arg++; /* first argument after 'format' (on stack) */

	while ( (c = *format++) != 0 )
	{
		if ( c != '%' )
		{
			k_stdout->print_char ( c, KERNEL_FONT );
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
					k_stdout->print_char ( (int) *p++,
							       KERNEL_FONT );
				break;

			case 's':
				p = *arg++;
				if ( !p )
					p = "(null)";

				while ( *p )
					k_stdout->print_char ( (int) *p++,
							       KERNEL_FONT );
				break;

			default: /* assuming 'c' */
				k_stdout->print_char ( *((int *) arg++),
						       KERNEL_FONT );
				break;
			}
		}
	}

	return 0;
}
