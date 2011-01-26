/*! Messages and signals */

#include <api/messages.h>
#include <api/thread.h>
#include <api/time.h>
#include <api/stdio.h>

char PROG_HELP[] = "Messaging example";

#define CONSUMERS	2
#define PRODUCERS	3
#define MSGS_PER_THREAD	3

static time_t sleep = { .sec = 1, .nsec = 0 };
static int end_msgs = 0;
static msg_q msgq;

/* example signal handler */
static void signal_handler ( msg_t *msg )
{
	print ( "Signal handler: msg->type=%d, msg->data[0]=%d\n",
		msg->type, msg->data[0] );
}

/* consumer thread */
static void consumer ( void *param )
{
	int i, thr_no, mybuff[MSGS_PER_THREAD*PRODUCERS], items;
	uint8 msg_buf[sizeof(msg_t) + 1];
	msg_t *msg = (msg_t *) msg_buf;
	thread_t self;

	thread_msg_set ( 0, 0, signal_handler );

	thread_self ( &self );
	thr_no = (int) param;
	items = 0;

	receive_message ( MSG_THREAD, &self, msg, 0, 1, IPC_WAIT );

	print ( "Consumer %d starting\n", thr_no );

	while ( end_msgs < PRODUCERS )
	{
		receive_message ( MSG_QUEUE, &msgq, msg, 0, 1, IPC_WAIT );

		if ( msg->data[0] == 0 ) {
			end_msgs++;
			continue;
		}

		mybuff[items] = msg->data[0];

		print ( "Consumer %d: got %d\n", thr_no, mybuff[items] );

		delay ( &sleep );

		items++;
	}
	print ( "Consumer %d exiting, received: ", thr_no );
	for ( i = 0; i < items; i++ )
		print ( "%d ", mybuff[i] );
	print ( "\n" );

	/* release single 'fellow' consumer since no more data */
	msg->size = 1;
	msg->type = 0;
	msg->data[0] = 0;
	send_message ( MSG_QUEUE, &msgq, msg, 0 );
}

/* producer thread */
static void producer ( void *param )
{
	int i, thr_no;
	msg_t msg;
	thread_t self;

	thread_self ( &self );

	thr_no = (int) param;

	receive_message ( MSG_THREAD, &self, &msg, 0, 1, IPC_WAIT );
	print ( "Producer %d starting\n", thr_no );

	msg.type = 1;
	msg.size = 1;

	for ( i = 0; i < MSGS_PER_THREAD; i++ )
	{
		msg.data[0] = thr_no * 10 + i % 10;

		print ( "Producer %d: sends %d\n", thr_no, msg.data[0] );

		send_message ( MSG_QUEUE, &msgq, &msg, 0 );

		delay ( &sleep );
	}
	print ( "Producer %d exiting\n", thr_no );

	msg.data[0] = 0; /* completion message */
	send_message ( MSG_QUEUE, &msgq, &msg, 0 );
}

int messages ( char *args[] )
{
	thread_t thread[CONSUMERS + PRODUCERS];
	msg_t msg;
	int i;

	end_msgs = 0;

	if ( create_message_queue ( &msgq, 0 ) )
	{
		print ( "Error creating message queue!\n" );
		return -1;
	}

	for ( i = 0; i < CONSUMERS; i++ )
		create_thread ( consumer, (void *) i+1, THR_DEFAULT_PRIO - 1,
				&thread[i] );
	for ( i = 0; i < PRODUCERS; i++ )
		create_thread ( producer, (void *) i+1, THR_DEFAULT_PRIO - 1,
				&thread[CONSUMERS + i] );

	/* send startup message to all threads */
	msg.size = 1;
	msg.type = 5;
	msg.data[0] = 5;
	for ( i = 0; i < CONSUMERS + PRODUCERS; i++ )
		send_message ( MSG_THREAD, &thread[i], &msg, 0 );

	delay ( &sleep );
	delay ( &sleep );
	send_message ( MSG_SIGNAL, &thread[0], &msg, 0 );


	for ( i = 0; i < CONSUMERS + PRODUCERS; i++ )
		wait_for_thread ( &thread[i], IPC_WAIT );

	delete_message_queue ( &msgq );

	return 0;
}
