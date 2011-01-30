/*! Formated printing on console (using 'console_t' interface) */

#include "print.h"
#include <arch/console.h>
#include <lib/string.h>
#include <lib/types.h>

/*! Formated output to console (lightweight version of printf) */
int __print ( char *format, ... )
{
	char **arg = &format;
	int c;
	char buf[20];

	#define PRINT_CHAR(C)	arch_console_print_char ( C, KERNEL_FONT )

	if ( !format )
		return 0;

	arg++; /* first argument after 'format' */

	while ( (c = *format++) != 0 )
	{
		if ( c != '%' )
		{
			PRINT_CHAR ( c );
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
					PRINT_CHAR ( (int) *p++ );
				break;

			case 's':
				p = *arg++;
				if ( !p )
					p = "(null)";

				while ( *p )
					PRINT_CHAR ( (int) *p++ );
				break;

			default: /* assuming 'c' */
				PRINT_CHAR ( *((int *) arg++) );
				break;
			}
		}
	}

	return 0;
}
