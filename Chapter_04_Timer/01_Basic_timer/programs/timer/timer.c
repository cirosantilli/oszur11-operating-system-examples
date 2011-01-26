/*! Timer api testing */

#include <api/stdio.h>
#include <api/time.h>
#include <lib/types.h>

static volatile int num = 0;

static void alarm ( void *param )
{
	time_t t;
	int id = (int) param;

	num++;
	time_get ( &t );

	print ( "Iter(%d): %d at %d:%d\n", id, num, t.sec, t.nsec/1000000 );
}

int timer ()
{
	time_t t, t1;
	int i;

	print ( "Test: [%s:%s]\n", __FILE__, __FUNCTION__ );

	time_get ( &t );
	print ( "System time: %d:%d\n", t.sec, t.nsec/1000000 );

	t1.sec = 3;
	t1.nsec = 0;
	t.sec += 3;
	alarm_set ( &t, alarm, (void *) t1.sec, &t1, 0 );

	while ( num < 3 )
		;

	alarm_remove ();

	time_get ( &t );
	print ( "System time: %d:%d\n", t.sec, t.nsec / 1000000 );

	t1.sec = 1;
	t1.nsec = 0;
	for ( i = 0; i < 5; i++ )
	{
		delay ( &t1 );
		time_get ( &t );
		print ( "Iter %d (%d:%d)\n", i, t.sec, t.nsec / 1000000 );
	}

	print ( "Timer test completed\n" );

	alarm_remove ();

	return 0;
}
