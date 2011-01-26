/*! Hello world program */

#include <api/stdio.h>

char PROG_HELP[] = "Print 'Hello world'";

int hello_world ( char *args[] )
{
	print ( "Hello World!\n" );

	return 0;
}
