/*! Threads example */

#include <api/stdio.h>
#include <api/thread.h>
#include <api/time.h>

#define THR_NUM	3
#define ITERS	5

char PROG_HELP[] = "Thread demonstration example: create several threads that "
		   "perform simple iterations and print basic info.";

static time_t sleep;

/* example threads */
static void simple_thread ( void *param )
{
	int i, thr_no;

	thr_no = (int) param;

	print ( "Thread %d starting\n", thr_no );
	for ( i = 1; i <= ITERS; i++ )
	{
		print ( "Thread %d: iter %d\n", thr_no, i );
		delay ( &sleep );
	}
	print ( "Thread %d exiting\n", thr_no );
}

int threads ( char *args[] )
{
	thread_t thread[THR_NUM];
	int i;

	sleep.sec = 1;
	sleep.nsec = 0;

	for ( i = 0; i < THR_NUM; i++ )
		create_thread ( simple_thread, (void *) i, THR_DEFAULT_PRIO - 1,
				&thread[i] );

	delay ( &sleep );
	delay ( &sleep );

	cancel_thread ( &thread[0] );
	print ( "Thread 0 canceled!\n" );

	for ( i = 0; i < THR_NUM; i++ )
		wait_for_thread ( &thread[i], IPC_WAIT );

	return 0;
}
