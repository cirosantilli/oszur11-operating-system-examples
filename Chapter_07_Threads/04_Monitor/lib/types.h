/*! Data types and constants */

#pragma once

#include <arch/types.h>

#if __WORD_SIZE >= 32
typedef word_t	size_t;
typedef sword_t	ssize_t;
#else /* size_t must be 32 bits or more */
typedef uint32 size_t;
typedef uint32 ssize_t;
#endif

#define NULL		((void *) 0)

#define FALSE		0
#define TRUE		1

#define IPC_WAIT	1
#define IPC_NOWAIT	2

#define IPC_WAIT_ANY	4	/* wait for any occurence/bytes/% */
#define IPC_WAIT_ALL	8	/* wait for all requested occurences/bytes/% */

/*! stdout ------------------------------------------------------------------ */
#define KERNEL_FONT	( 1 << 0 )
#define USER_FONT	( 1 << 1 )

#define PRINTCHAR	( 1 << 2 )
#define CLEAR		( 1 << 3 )
#define GOTOXY		( 1 << 4 )

/*! stdin ------------------------------------------------------------------- */
#define ECHO_OFF	1
#define ECHO_ON		2
#define ONLY_ASCII	4
#define ONLY_LAST	8
#define RAW_KEYSTROKES	16
#define CLEAR_BUFFER	32

/*! serial port (UART) ------------------------------------------------------ */
/* commands */
#define UART_SETCONF	( 1 << 30 )	/* reconfigure port */
#define UART_GETCONF	( 1 << 31 )	/* read configuration */

/* parameters for configuring serial port */
typedef struct _uart_t_
{
	int	speed;		/* baud rate				*/
	int8	data_bits;	/* from 5 to 8				*/
	int8	parity;		/* PARITY_+ NONE/ODD/EVEN/MARK/SPACE	*/
	int8	stop_bit;	/* STOPBIT_1 or STOPBIT_15		*/
	int8	mode;		/* UART_BYTE or UART_STREAM		*/
}
uart_t;

#include <arch/devices/uart.h>

/*! Time -------------------------------------------------------------------- */
/* alarm types */
#define ALARM_PERIODIC	4

/* system time format */
typedef struct _time_t_
{
	int sec;	/* seconds */
	int nsec;	/* nanoseconds */
}
time_t;

/* alarm structure with basic alarm informations */
typedef struct _alarm_t_
{
	time_t exp_time;	/* expiration time (alarm activation),
				   absolute value, comparing to system time */

	void (*action) (void *);/* activation function, accept pointer
				   as parameter */
	void *param;		/* pointer passed to act. function */

	unsigned int flags;	/* defines additional alarm behavior */

	time_t period;		/* if timer is periodic, this is period */
}
alarm_t;


/*! Thread ------------------------------------------------------------------ */
typedef struct _thread_t_
{
	void *thread;
	int thr_id;
}
thread_t;


/*! Semaphore --------------------------------------------------------------- */
typedef struct _sem_t_
{
	void *ptr;
}
sem_t;


/*! Monitor and monitor queue (conditional variable) ------------------------ */
typedef struct _monitor_t_
{
	void *ptr;
}
monitor_t;

typedef struct _monitor_q_
{
	void *ptr;
}
monitor_q;


/*! Short functions - time_t manipulation ----------------------------------- */

/*!
 * Calculate a = a + b (both represent time)
 * \param a First time
 * \param b Second time
 */
static inline void time_add ( time_t *a, time_t *b )
{
	a->sec = a->sec + b->sec;
	a->nsec = a->nsec + b->nsec;

	if ( a->nsec > 1000000000L )
	{
		a->sec++;
		a->nsec -= 1000000000L;
	}
}

/*!
 * Calculate a = a - b (both represent time)
 * \param a First time
 * \param b Second time
 * NOTE: assumes a > b, does not check this!
 */
static inline void time_sub ( time_t *a, time_t *b )
{
	if ( a->nsec >= b->nsec )
	{
		a->sec -= b->sec;
		a->nsec -= b->nsec;
	}
	else {
		a->sec -= b->sec + 1;
		a->nsec += 1000000000L - b->nsec;
	}
}

/*!
 * Compare times
 * \param a First time
 * \param b Second time
 * \returns -1 when a < b, 0 when a == b, 1 when a > b
 */
static inline int time_cmp (time_t *a, time_t *b)
{
	if ( a->sec < b->sec )
		return -1;

	if ( a->sec > b->sec )
		return 1;

	/* a->sec == b->sec */

	if ( a->nsec < b->nsec )
		return -1;

	if ( a->nsec > b->nsec )
		return 1;

	/* a->nsec == b->nsec */
	return 0;
}

