/*! User threads example */

#include "uthread.h"

#include <api/stdio.h>

char PROG_HELP[] = "Threads created and managed in user space - kernel sees"
		   " only single thread";

void first  ( void *param );
void second ( void *param );
void third  ( void *param );

volatile static int thr_num;

int user_threads ( char *args[] )
{
	uthread_t *thread;

	uthreads_init ();

	thread = create_uthread ( first, (void *) 1 );
	thread = create_uthread ( second, (void *) 2 );
	thread = create_uthread ( third, (void *) 3 );
	thr_num = 3;

	while (	thr_num > 0 )
		uthread_yield ();

	return 0;
}

/* example threads */
void first ( void *param )
{
	int i;

	print ( "First thread starting, param %x\n", param );
	for ( i = 0; i < 3; i++ )
	{
		print ( "First thread, iter %d\n", i );
		uthread_yield ();
	}
	print ( "First thread exiting\n" );

	thr_num--;
}

void second ( void *param )
{
	int i;

	print ( "Second thread starting, param %x\n", param );
	for ( i = 0; i < 3; i++ )
	{
		print ( "Second thread, iter %d\n", i );
		uthread_yield ();
	}
	print ( "Second thread exiting\n" );
	thr_num--;
}

void third ( void *param )
{
	int i;

	print ( "Third thread starting, param %x\n", param );
	for ( i = 0; i < 3; i++ )
	{
		print ( "Third thread, iter %d\n", i );
		uthread_yield ();
	}
	print ( "Third thread exiting\n" );

	thr_num--;
}
