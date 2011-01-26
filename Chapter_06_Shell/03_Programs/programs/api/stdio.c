/*! Printing on stdout, reading from stdin */

#include "stdio.h"
#include <api/prog_info.h>
#include <lib/string.h>
#include <lib/types.h>
#include <api/syscalls.h>

extern prog_info_t pi;

/*! Get input from "standard input" */
inline int get_char ()
{
	int c = 0;

	sys__device_recv ( (void *) &c, 1, ONLY_ASCII, pi.stdin );

	return c;
}

/*! Clear whole screen */
inline int clear_screen ()
{
	return sys__device_send ( NULL, 0, CLEAR, pi.stdout );
}

/*! Move cursor to (x,y) position (x is vertical, y horizontal) */
inline int goto_xy ( int x, int y )
{
	int p[2];

	p[0] = x;
	p[1] = y;

	return sys__device_send ( &p, 2 * sizeof (int), GOTOXY, pi.stdout );
}

/*! Print single character to current cursor location with defined color */
inline int print_char ( int c, int attr )
{
	int p[2];

	p[0] = c;
	p[1] = attr;

	return sys__device_send ( p, 2 * sizeof(int), PRINTCHAR, pi.stdout );
}

/*! Formated output to stdout (lightweight version of printf) */
int print ( char *format, ... )
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
			print_char ( c, USER_FONT );
		}
		else {
			char *p;

			c = *format++;
			switch ( c )
			{
			case 'd':
			case 'u':
			case 'x':
			case 'X':
				itoa ( buf, c, *((int *) arg++) );
				p = buf;
				while ( *p )
					print_char ( (int) *p++, USER_FONT );
				break;

			case 's':
				p = *arg++;
				if ( !p )
					p = "(null)";

				while ( *p )
					print_char ( (int) *p++, USER_FONT );
				break;

			default: /* assuming 'c' */
				print_char ( *((int *) arg++), USER_FONT );
				break;
			}
		}
	}

	return 0;
}
