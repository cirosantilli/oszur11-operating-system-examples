/*! Thread management */
#define _KERNEL_

#define _K_THREAD_C_
#include "thread.h"

#include <arch/interrupts.h>
#include <kernel/memory.h>
#include <kernel/devices.h>
#include <kernel/kprint.h>
#include <kernel/errno.h>
#include <lib/bits.h>
#include <lib/list.h>
#include <lib/string.h>
#include <arch/processor.h>
#include <api/prog_info.h>

static list_t all_threads; /* all threads */

static kthread_t *active_thread = NULL; /* active thread */

static kthread_q ready_q[PRIO_LEVELS]; /* ready threads organized by priority */

extern prog_info_t pi;

/*! initialize thread structures and create idle thread */
void k_threads_init ()
{
	extern kdevice_t *u_stdin, *u_stdout;
	kthread_t *kthr;
	int prio;

	list_init ( &all_threads );

	/* queue for ready threads is empty */
	init_ready_list ();

	/* setup programs */
	pi.stdin = u_stdin;
	pi.stdout = u_stdout;

	pi.heap = kmalloc ( PROG_HEAP_SIZE );
	pi.heap_size = PROG_HEAP_SIZE;

	prio = pi.prio;
	if ( !prio )
		prio = THR_DEFAULT_PRIO;

	/* idle thread */
	kthr = k_create_thread ( idle_thread, NULL, NULL, 0, NULL, 0, 1 );

	/* first "user" thread */
	k_create_thread (pi.init, NULL, pi.exit, prio, NULL, 0, 1 );

	active_thread = NULL;

	k_schedule_threads ();
}

/*!
 * Create new thread
 * \param start_func Starting function for new thread
 * \param param Parameter sent to starting function
 * \param exit_func Thread will call this function when it leaves 'start_func'
 * \param prio Thread priority
 * \param stack Address of thread stack (if not NULL)
 * \param stack_size Stack size
 * \param run Move thread descriptor to ready threads?
 * \returns Pointer to descriptor of created kernel thread
 */
kthread_t *k_create_thread ( void *start_func, void *param, void *exit_func,
			     int prio, void *stack, size_t stack_size, int run )
{
	kthread_t *kthr;

	/* if stack is not defined */
	if ( !stack || !stack_size )
	{
		if ( !stack_size )
			stack_size = DEFAULT_THREAD_STACK_SIZE;

		stack = kmalloc ( stack_size );
	}
	ASSERT ( stack && stack_size );

	kthr = kmalloc ( sizeof (kthread_t) ); /* thread descriptor */
	ASSERT ( kthr );

	/* initialize thread descriptor */
	kthr->id = k_new_unique_id ();

	kthr->state = THR_STATE_PASSIVE;

	if ( prio < 0 ) prio = 0;
	if ( prio >= PRIO_LEVELS ) prio = PRIO_LEVELS - 1;
	kthr->sched.prio = prio;

	arch_create_thread_context ( &kthr->context, start_func, param,
				     exit_func, stack, stack_size );
	kthr->queue = NULL;
	kthr->exit_status = 0;
	k_threadq_init ( &kthr->join_queue );
	kthr->ref_cnt = 0;

	if ( run ) {
		move_to_ready ( kthr );
		kthr->ref_cnt = 1;
	}

	kthr->stack = stack;
	kthr->stack_size = stack_size;

	list_append ( &all_threads, kthr, &kthr->all );

	return kthr;
}

/*!
 * Select highest priority ready thread as active
 * - if different from current, move current into ready queue (id not NULL) and
 *   move selected thread from ready queue to active queue
 */
void k_schedule_threads ()
{
	int first;
	kthread_t *curr, *next;
	context_t dummy, *curr_cntx;
	arch_context_t cntx;

	curr = active_thread;

	first = get_top_ready ();

	/* must be an thread to return to, 'curr' or first from 'ready' */
	ASSERT ( ( curr && curr->state == THR_STATE_ACTIVE ) || first >= 0 );

	if ( !curr || curr->state != THR_STATE_ACTIVE ||
		first >= curr->sched.prio )
	{
		/* change active thread, move current to ready queue */
		if ( curr && curr->state == THR_STATE_ACTIVE )
			move_to_ready ( curr );

		next = k_threadq_remove ( &ready_q[first], NULL );
		ASSERT ( next );

		/* no more ready threads in list? */
		if ( k_threadq_get( &ready_q[first] ) == NULL )
			clear_got_ready ( first );

		active_thread = next;
		active_thread->state = THR_STATE_ACTIVE;
		active_thread->queue = NULL;
	}

	if ( curr != active_thread )
	{
		if ( curr )
		{
			curr_cntx = &curr->context;
		}
		else {
			dummy.context = &cntx;
			curr_cntx = &dummy;
		}

		/* switch to newly selected thread */
		arch_switch_to_thread ( curr_cntx, &active_thread->context );
	}
	/* else => continue with current thread */
}

/*! operations on thread queues (blocked threads) --------------------------- */

/*!
 * Put given thread or active thread (when kthr == NULL) into queue 'q_id'
 * - if kthr != NULL, thread must not be in any list and 'k_schedule_threads'
 *   should follow this call before exiting from kernel!
 */
void k_enqueue_thread ( kthread_t *kthr, kthread_q *q )
{
	ASSERT ( ( kthr || active_thread ) && q );

	if ( !kthr )
		kthr = active_thread;

	kthr->state = THR_STATE_WAIT;
	kthr->queue = q;

	k_threadq_append ( kthr->queue, kthr );
}

/*!
 * Release single thread from given queue (if queue not empty)
 * \param q Queue
 * \returns 1 if thread was released, 0 if queue was empty
 */
int k_release_thread ( kthread_q *q )
{
	kthread_t *kthr;

	ASSERT ( q );

	kthr = k_threadq_remove ( q, NULL );

	if ( kthr )
	{
		move_to_ready ( kthr );
		return 1;
	}
	else {
		return 0;
	}
}

/*!
 * Release all threads from given queue (if queue not empty)
 * \param q Queue
 * \returns number of thread released, 0 if queue was empty
 */
int k_release_all_threads ( kthread_q *q )
{
	kthread_t *kthr;
	int cnt = 0;

	ASSERT ( q );

	do {
		kthr = k_threadq_remove ( q, NULL );

		if ( kthr )
		{
			move_to_ready ( kthr );
			cnt++;
		}
	}
	while ( kthr );

	return cnt;
}

/*! Ready thread list (multi-level organized; one level per priority) ------- */

/* masks for fast searching for highest priority ready thread */
#define RDY_MASKS ( ( PRIO_LEVELS + sizeof (word_t) - 1 ) / sizeof (word_t) )
static word_t rdy_mask[ RDY_MASKS ];

/*! Initialize ready thread list */
static void init_ready_list ()
{
	int i;

	/* queue for ready threads is empty */
	for ( i = 0; i < PRIO_LEVELS; i++ )
		k_threadq_init ( &ready_q[i] );

	for ( i = 0; i < RDY_MASKS; i++ )
		rdy_mask[i] = 0;

}

/*! Find and return priority of highest priority thread in ready list */
static int get_top_ready ()
{
	int i, first = -1;

	for ( i = RDY_MASKS - 1; i >= 0; i-- )
		if ( rdy_mask[i] )
		{
			first = i * sizeof (word_t) + msb_index ( rdy_mask[i] );
			break;
		}

	return first;
}

/*! Mark ready list for given priority (level) as non-empty */
static void set_got_ready ( int index )
{
	int i, j;

	i = index / sizeof (word_t);
	j = index % sizeof (word_t);

	rdy_mask[i] |= (word_t) ( 1 << j );
}

/*! Mark ready list for given priority (level) as empty */
static void clear_got_ready ( int index )
{
	int i, j;

	i = index / sizeof (word_t);
	j = index % sizeof (word_t);

	rdy_mask[i] &= ~( (word_t) ( 1 << j ) );
}

/*! Move given thread (its descriptor) to ready threads */
static void move_to_ready ( kthread_t *kthr )
{
	kthr->state = THR_STATE_READY;
	kthr->queue = &ready_q[kthr->sched.prio];

	k_threadq_append ( kthr->queue, kthr );
	set_got_ready ( kthr->sched.prio );
}

/*! Idle thread ------------------------------------------------------------- */

/*! Idle thread starting (and only) function */
static void idle_thread ( void *param )
{
	while (1)
		suspend ();
}


/*! >>> (Syscall) interface to threads -------------------------------------- */

/*!
 * Create new thread
 * \param func Starting function
 * \param param Parameter for starting function
 * \param prio Priority for new thread
 * \param thr_desc User level thread descriptor
 * (parameters are on calling thread stack)
 */
int sys__create_thread ( void *func, void *param, int prio, thread_t *thread )
{
	kthread_t *kthr;

	disable_interrupts ();

	kthr = k_create_thread ( func, param, pi.exit, prio, NULL, 0, 1 );

	if ( thread )
	{
		thread->thread = kthr;
		thread->thr_id = kthr->id;
	}

	SET_ERRNO ( SUCCESS );

	k_schedule_threads ();

	enable_interrupts ();

	EXIT ( SUCCESS );
}

/*! Internal function form removing (freeing) thread descriptor */
static void k_remove_thread_descriptor ( kthread_t *kthr )
{
	k_free_unique_id ( kthr->id );
	kthr->id = 0;

	list_remove ( &all_threads, FIRST, &kthr->all );

	kfree ( kthr );
}

static void k_remove_thread ( kthread_t *kthr )
{
	kthr->state = THR_STATE_PASSIVE;
	kthr->ref_cnt--;

	/* release thread stack */
	if ( kthr->stack )
		kfree ( kthr->stack );

	if ( !kthr->ref_cnt )
		k_remove_thread_descriptor ( kthr );
	else
		k_release_all_threads ( &kthr->join_queue );
}

/*!
 * End current thread (exit from it)
 * \param status Exit status number
 */
int sys__thread_exit ( int status )
{
	disable_interrupts ();

	active_thread->exit_status = status;

	k_remove_thread ( active_thread );

	active_thread = NULL;

	k_schedule_threads ();

	/* thread never reaches to here! */
	enable_interrupts ();

	return 0;
}

/*! Wait for thread termination
 * \param thread Thread descriptor (user level descriptor)
 * \param wait Wait if thread not finished (!=0) or not (0)?
 * \returns 0 if thread already gone; -1 if not finished and 'wait' not set;
 *          'thread exit status' otherwise
 */
int sys__wait_for_thread ( thread_t *thread, int wait )
{
	kthread_t *kthr;
	int ret_value = 0;

	ASSERT_ERRNO_AND_EXIT ( thread && thread->thread, E_INVALID_HANDLE );

	disable_interrupts ();

	kthr = thread->thread;

	if ( kthr->id != thread->thr_id ) /* at 'kthr' is now something else */
	{
		ret_value = -SUCCESS;
		SET_ERRNO ( SUCCESS );
	}
	else if ( kthr->state != THR_STATE_PASSIVE && !wait )
	{
		ret_value = -E_NOT_FINISHED;
		SET_ERRNO ( E_NOT_FINISHED );
	}
	else if ( kthr->state != THR_STATE_PASSIVE )
	{
		kthr->ref_cnt++;

		ret_value = -E_RETRY; /* retry (collect thread status) */
		SET_ERRNO ( E_RETRY );

		k_enqueue_thread ( NULL, &kthr->join_queue );

		k_schedule_threads ();
	}
	else {
		/* kthr->state == THR_STATE_PASSIVE, but thread descriptor still
		   not freed - some thread still must collect its status */
		SET_ERRNO ( SUCCESS );
		ret_value = kthr->exit_status;

		kthr->ref_cnt--;

		if ( !kthr->ref_cnt )
			k_remove_thread_descriptor ( kthr );
	}

	enable_interrupts ();

	return ret_value;
}

/*!
 * Cancel some other thread
 * \param thread Thread descriptor (user)
 */
int sys__cancel_thread ( thread_t *thread )
{
	kthread_t *kthr;
	int ret_value = 0;

	ASSERT_ERRNO_AND_EXIT ( thread && thread->thread, E_INVALID_HANDLE );

	disable_interrupts ();

	kthr = thread->thread;

	if ( kthr->id != thread->thr_id )
	{
		ret_value = SUCCESS; /* thread is already finished */
	}
	else {

		/* remove thread from queue where its descriptor is */
		switch ( kthr->state )
		{
		case THR_STATE_PASSIVE:
			ret_value = SUCCESS; /* thread is already finished */
			break;

		case THR_STATE_READY:
		case THR_STATE_WAIT:
			SET_ERRNO ( SUCCESS );
			/* remove target 'thread' from its queue */
			k_threadq_remove ( kthr->queue, kthr );

			if ( kthr->state == THR_STATE_READY &&
			k_threadq_get( &ready_q[kthr->sched.prio] ) == NULL )
				clear_got_ready ( kthr->sched.prio );

			kthr->exit_status = -E_CANCELED;
			k_remove_thread ( kthr );

			ret_value = SUCCESS; /* thread is already finished */
			break;

		case THR_STATE_ACTIVE:
		default: /* thread descriptor is corrupted ! */
			ret_value = E_INVALID_HANDLE;
			break;
		}
	}

	enable_interrupts ();

	return ret_value;
}

/*! Return calling thread descriptor
 * \param thread Thread descriptor (user level descriptor)
 * \returns 0
 */
int sys__thread_self ( thread_t *thread )
{
	ASSERT_ERRNO_AND_EXIT ( thread, E_INVALID_HANDLE );

	thread->thread = active_thread;
	thread->thr_id = active_thread->id;

	EXIT ( SUCCESS );
}

/*!
 * Display info on threads
 */
int k_thread_info ()
{
	kthread_t *kthr;
	int i = 1;

	kprint ( "Threads info\n" );

	kprint ( "\t[this]\tid=%d prio=%d, state=%d, ret_val=%d\n",
		  active_thread->id, active_thread->sched.prio,
		  active_thread->state, active_thread->exit_status );

	kthr = list_get ( &all_threads, FIRST );
	while ( kthr )
	{
		kprint ( "\t[%d]\tid=%d prio=%d, state=%d, ret_val=%d\n",
			  i++, kthr->id, kthr->sched.prio, kthr->state,
			  kthr->exit_status );

		kthr = list_get_next ( &kthr->all );
	}

	return 0;
}

/*! Set and get current thread error status */
int sys__set_errno ( int errno )
{
	active_thread->errno = errno;

	return 0;
}
int sys__get_errno ()
{
	return active_thread->errno;
}

/*! <<< Interface to threads ------------------------------------------------ */


/*! Get-ers, Set-ers and misc ----------------------------------------------- */

inline void *k_get_active_thread ()
{
	return (void *) active_thread;
}
inline void *k_get_thread_context ( kthread_t *thread )
{
	return &thread->context;
}
inline void *k_get_active_thread_context ()
{
	return &active_thread->context;
}
inline int k_get_thread_prio ( kthread_t *thread )
{
	return thread->sched.prio;
}
inline int k_get_active_thread_prio ()
{
	return active_thread->sched.prio;
}

/*! thread queue manipulation */
inline void k_threadq_init ( kthread_q *q )
{
	list_init ( &q->q );
}
inline void k_threadq_append ( kthread_q *q, kthread_t *kthr )
{
	list_append ( &q->q, kthr, &kthr->ql );
}
inline kthread_t *k_threadq_remove ( kthread_q *q, kthread_t *kthr )
{
	if ( kthr )
		return list_remove ( &q->q, FIRST, &kthr->ql );
	else
		return list_remove ( &q->q, FIRST, NULL );
}
inline kthread_t *k_threadq_get ( kthread_q *q )
{
	return list_get ( &q->q, FIRST );
}
inline kthread_t *k_threadq_get_next ( kthread_t *kthr )
{
	return list_get_next ( &kthr->ql );
}

/*! Temporary storage for blocked thread (save specific context before wait) */
inline void k_set_thread_qdata ( kthread_t *kthr, void *qdata )
{
	if ( !kthr )
		kthr = active_thread;

	kthr->qdata = qdata;
}
inline void *k_get_thread_qdata ( kthread_t *kthr )
{
	if ( !kthr )
		return active_thread->qdata;
	else
		return kthr->qdata;
}

/*! errno */
inline void k_set_errno ( int error_number )
{
	active_thread->errno = error_number;
}
inline void k_set_thread_errno ( kthread_t *kthr, int error_number )
{
	kthr->errno = error_number;
}
inline int k_get_errno ()
{
	return active_thread->errno;
}
inline int k_get_thread_errno ( kthread_t *kthr )
{
	return kthr->errno;
}
