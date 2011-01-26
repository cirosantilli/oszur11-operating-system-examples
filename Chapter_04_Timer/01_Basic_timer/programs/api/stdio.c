/*! Printing on stdout, reading from stdin */

#include "stdio.h"
#include <arch/console.h>
#include <lib/print.h>

extern console_t *u_stdout; /* defined in kernel/startup.c */


/*! Clear whole screen */
inline int clear_screen ()
{
	return u_stdout->clear ();
}
/*! Move cursor to (x,y) position (x is vertical, y horizontal) */
inline int goto_xy ( int x, int y )
{
	return u_stdout->gotoxy ( x, y );
}

/*! Print single character to current cursor location with defined color */
inline int print_char ( int c, int attr )
{
	return u_stdout->print_char ( c, attr );
}

/*! Formated output to console (lightweight version of printf) */
int print ( char *format, ... )
{
	return __print ( u_stdout, USER_FONT, &format );
}
