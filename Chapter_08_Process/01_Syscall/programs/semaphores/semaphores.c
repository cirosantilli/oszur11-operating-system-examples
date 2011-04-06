/*! Semaphore example (threads) */

#include <api/stdio.h>
#include <api/thread.h>
#include <api/semaphore.h>
#include <api/time.h>

char PROG_HELP[] = "Thread example (semaphore): producer consumer example.";

#define CONSUMERS	2
#define PRODUCERS	3
#define BUFF_SIZE	5

static time_t sleep;
static int end_msgs = 0;

static int buffer[BUFF_SIZE], in, out;

static sem_t filled, empty, crit1, crit2;

/* consumer thread */
static void consumer ( void *param )
{
	int i, thr_no, mybuff[BUFF_SIZE*PRODUCERS], items;

	thr_no = (int) param;
	items = 0;

	print ( "Consumer %d starting\n", thr_no );

	while ( end_msgs < PRODUCERS )
	{
		sem_wait ( &filled );
		sem_wait ( &crit1 );

		if ( end_msgs >= PRODUCERS )
		{
			sem_post ( &crit1 );
			sem_post ( &empty );
			break;
		}

		mybuff[items] = buffer[out];
		if ( ++out >= BUFF_SIZE )
			out %= BUFF_SIZE;

		if ( !mybuff[items] )
			end_msgs++;

		print ( "Consumer %d: got %d\n", thr_no, mybuff[items++] );

		sem_post ( &crit1 );
		sem_post ( &empty );

		delay ( &sleep );
	}

	print ( "Consumer %d exiting, received: ", thr_no );
	for ( i = 0; i < items; i++ )
		print ( "%d ", mybuff[i] );
	print ( "\n" );

	sem_post ( &filled ); /* release 'fellow' consumer since no more data */
}

/* producer thread */
static void producer ( void *param )
{
	int i, thr_no, data;

	thr_no = (int) param;

	print ( "Producer %d starting\n", thr_no );

	for ( i = 1; i < BUFF_SIZE; i++ )
	{
		sem_wait ( &empty );
		sem_wait ( &crit2 );

		data = ( i < BUFF_SIZE - 1 ? thr_no * 10 + i % 10 : 0 );

		print ( "Producer %d: sends %d\n", thr_no, data );

		buffer[in] = data;
		if ( ++in >= BUFF_SIZE )
			in %= BUFF_SIZE;

		sem_post ( &crit2 );
		sem_post ( &filled );

		delay ( &sleep );
	}
	print ( "Producer %d exiting\n", thr_no );
}

int semaphores ( char *args[] )
{
	thread_t thread[CONSUMERS + PRODUCERS];
	int i;

	sleep.sec = 1;
	sleep.nsec = 0;

	sem_init ( &filled, 0 );
	sem_init ( &empty, BUFF_SIZE );
	sem_init ( &crit1, 1 );
	sem_init ( &crit2, 1 );
	in = out = 0;
	end_msgs = 0;

	for ( i = 0; i < CONSUMERS; i++ )
		create_thread ( consumer, (void *) i+1, THR_DEFAULT_PRIO - 1,
				&thread[i] );
	for ( i = 0; i < PRODUCERS; i++ )
		create_thread ( producer, (void *) i+1, THR_DEFAULT_PRIO - 1,
				&thread[CONSUMERS + i] );

	for ( i = 0; i < CONSUMERS + PRODUCERS; i++ )
		wait_for_thread ( &thread[i], IPC_WAIT );

	sem_destroy ( &filled );
	sem_destroy ( &empty );
	sem_destroy ( &crit1 );
	sem_destroy ( &crit2 );

	return 0;
}
