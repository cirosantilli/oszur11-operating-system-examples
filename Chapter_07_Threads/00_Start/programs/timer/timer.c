/*! Timer api testing */

#include <api/stdio.h>
#include <api/time.h>
#include <lib/types.h>

char PROG_HELP[] = "Timer interface demonstration: periodic timer activations.";

static time_t t0;

static void alarm_n ( void *param )
{
	int num;
	time_t t;

	num = (int) param;
	time_get ( &t );
	time_sub ( &t, &t0 );

	print ( "[%d:%d] Alarm %d (every %d seconds)\n",
		t.sec, t.nsec/1000000, num, num );
}

int timer ( char *args[] )
{
	time_t t, t1, t2;
	void *alarm1, *alarm2;

	print ( "Test: [%s:%s]\n", __FILE__, __FUNCTION__ );

	time_get ( &t );
	print ( "System time: %d:%d\n", t.sec, t.nsec/1000000 );
	t.nsec = 0;
	t0 = t;

	t1.sec = 3;
	t1.nsec = 0;
	t.sec += 3;
	alarm1 = alarm_set ( NULL, &t, alarm_n, (void *) t1.sec, &t1, 0 );

	t2.sec = 5;
	t2.nsec = 0;
	t.sec += 2;
	alarm2 = alarm_set ( NULL, &t, alarm_n, (void *) t2.sec, &t2, 0 );

	t.sec = 16;
	delay ( &t );

	time_get ( &t );
	print ( "System time: %d:%d\n", t.sec, t.nsec / 1000000 );
	print ( "Changing first alarm (every 3) to (every 7)\n" );

	t2.sec = 7;
	t.sec += 7;
	t.nsec = t0.nsec;

	alarm_set ( alarm1, &t, alarm_n, (void *) t2.sec, &t2, 0 );

	t.sec = 20;
	delay ( &t );

	time_get ( &t1 );
	print ( "System time: %d:%d\n", t1.sec, t1.nsec / 1000000 );

	alarm_remove ( alarm1 );
	alarm_remove ( alarm2 );

	return 0;
}
