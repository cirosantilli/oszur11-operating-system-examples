/*! Monitor example (threads) */

#include <api/stdio.h>
#include <api/thread.h>
#include <api/monitor.h>
#include <api/time.h>

char PROG_HELP[] = "Thread example (monitor): The dining philosophers problem.";

#define PHNUM	5

static time_t eat, think;
static int terminate_simulation;

static int stick[PHNUM];
static char phs[PHNUM];

static monitor_t m;
static monitor_q q[PHNUM];

/* philosopher thread */
static void philosopher ( void *param )
{
	int phil, lstick, rstick, lphil, rphil;

	phil = (int) param;
	lstick = phil;
	rstick = ( lstick + 1 ) % PHNUM;
	lphil = ( phil + PHNUM - 1 ) % PHNUM;
	rphil = ( phil + 1 ) % PHNUM;

	print ( "%s - Philosopher %d thinking\n", phs, phil );

	while ( !terminate_simulation )
	{
		delay ( &think );

		monitor_lock ( &m );
			phs[phil] = '-';
			while ( stick[lstick] || stick[rstick] )
				monitor_wait ( &m, &q[phil] );
			stick[lstick] = stick[rstick] = TRUE;
			phs[phil] = 'X';
			print ( "%s - Philosopher %d eating\n", phs, phil );
		monitor_unlock ( &m );

		delay ( &eat );

		monitor_lock ( &m );
			stick[lstick] = stick[rstick] = FALSE;
			phs[phil] = 'O';
			print ( "%s - Philosopher %d thinking\n", phs, phil );
		monitor_unlock ( &m );

		monitor_signal ( &q[lphil] );
		monitor_signal ( &q[rphil] );
	}
}

int monitors ( char *args[] )
{
	thread_t thread[PHNUM];
	time_t sim_time;
	int i;

	eat.sec = 3;
	eat.nsec = 0;
	think.sec = 3;
	think.nsec = 0;
	sim_time.sec = 30;
	sim_time.nsec = 0;

	terminate_simulation = 0;

	monitor_init ( &m );

	for ( i = 0; i < PHNUM; i++ )
	{
		stick[i] = 0;
		phs[i] = 'O';
		monitor_queue_init ( &q[i] );
	}

	for ( i = 0; i < PHNUM; i++ )
		create_thread ( philosopher, (void *) i, THR_DEFAULT_PRIO - 1,
				&thread[i] );

	delay ( &sim_time );

	terminate_simulation = 1;

	for ( i = 0; i < PHNUM; i++ )
		wait_for_thread ( &thread[i], IPC_WAIT );

	monitor_destroy ( &m );

	for ( i = 0; i < PHNUM; i++ )
		monitor_queue_destroy ( &q[i] );

	return 0;
}
