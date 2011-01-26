/*! Print program arguments */

#include <api/stdio.h>

char PROG_HELP[] = "Print given command line arguments";

int arguments ( char *argv[] )
{
	int i;

	print ( "Program arguments: " );
	for ( i = 0; argv[i]; i++ )
		print ( "[%s] ", argv[i] );

	return 0;
}
