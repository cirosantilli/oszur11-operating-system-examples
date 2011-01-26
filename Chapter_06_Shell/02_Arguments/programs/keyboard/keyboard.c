/*! Keyboard api testing */

#include <api/stdio.h>
#include <api/time.h>
#include <lib/types.h>

int keyboard ( char *args[] )
{
	int key;
	time_t t = { .sec = 0, .nsec = 100000000 };

	print ( "Test: [%s:%s]\n", __FILE__, __FUNCTION__ );
	print ( "Test keyboard ('.' for end)\n" );

	do {
		if ( ( key = get_char () ) )
			print ( "Got: %c (%d)\n", key, key );
		else
			delay ( &t );
	}
	while ( key != '.' );

	print ( "End of keyboard test\n" );

	return 0;
}
