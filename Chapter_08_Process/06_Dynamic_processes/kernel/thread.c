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

#ifdef	MESSAGES
#include <kernel/messages.h>
#endif

static list_t all_threads; /* all threads */

static kthread_t *active_thread = NULL; /* active thread */

static kthread_q ready_q[PRIO_LEVELS]; /* ready threads organized by priority */

kprocess_t kernel_proc; /* kernel process (currently only for idle thread) */
static list_t procs; /* list of all processes */

/*! initialize thread structures and create idle thread */
void k_threads_init ()
{
	kthread_t *kthr;

	list_init ( &all_threads );
	list_init ( &procs );

	/* queue for ready threads is empty */
	init_ready_list ();

	/* initially create 'idle thread' */
	kernel_proc.prog = NULL;
	kernel_proc.stack_pool = NULL;
	kernel_proc.m.start = NULL;
	kernel_proc.m.size = (size_t) 0xffffffff;

	kthr = k_create_thread ( idle_thread, NULL, NULL, 0, NULL, 0, 1,
				&kernel_proc );

	active_thread = NULL;

	k_schedule_threads ();
}

/*!
 * Start program defined by 'prog' (loaded as module) as new process:
 * - initialize environment (stack area for threads, stdin, stdout) and start
 *   it's first thread
 * \param prog_name Program name (as given with module)
 * \param param Command line arguments for starting thread (if not NULL)
 * \param prio Priority for starting thread
 * \returns Pointer to descriptor of created process
 */
kthread_t *k_proc_start ( char *prog_name, void *param, int prio )
{
	extern kdevice_t *u_stdin, *u_stdout;
	extern list_t progs;
	kprog_t *prog;
	kprocess_t *proc;
	kthread_t *kthr;
	char **args = NULL, *arg, *karg, **kargs;
	size_t argsize;
	int i;

	prog = list_get ( &progs, FIRST );
	while ( prog && strcmp ( prog->prog_name, prog_name ) )
		prog = list_get_next ( &prog->all );

	if ( !prog )
		return NULL;

	/* create new process */
	proc = kmalloc ( sizeof ( kprocess_t) );
	ASSERT ( proc );

	proc->prog = prog;
	proc->m.size = prog->m.size + prog->pi->heap_size + prog->pi->stack_size;

	proc->m.start = proc->pi = kmalloc ( proc->m.size );

	if ( !proc->pi )
	{
		kprint ( "Not enough memory! (%d)\n", proc->m.size );
		kfree ( proc->pi );
		return NULL;
	}

	/* copy code and data */
	memcpy ( proc->pi, prog->pi, prog->m.size );

	/* define heap and stack */
	proc->pi->heap = (void *) proc->pi + prog->m.size;
	proc->pi->stack = proc->pi->heap + prog->pi->heap_size;
	memset (proc->pi->heap, 0, prog->pi->heap_size + prog->pi->stack_size);
	proc->m.start = proc->pi;

	proc->pi->stdin = u_stdin;
	proc->pi->stdout = u_stdout;

	/* initialize memory pool for threads stacks */
	proc->stack_pool = ffs_init ( proc->pi->stack, prog->pi->stack_size );

	/* set addresses in process header to relative addresses */
	proc->pi->heap = (void *) prog->m.size;
	proc->pi->stack = proc->pi->heap + prog->pi->heap_size;
	proc->pi->end_adr = proc->pi->stack + prog->pi->stack_size;

	proc->thr_count = 0;

	if ( !prio )
		prio = proc->pi->prio;
	if ( !prio )
		prio = THR_DEFAULT_PRIO;

	if ( param ) /* have arguments? */
	{
		/* copy command line arguments from kernel space to process;
		   (use process stack space for arguments) */
		kargs = param;
		for ( i = 0; kargs[i]; i++ ) ;
		argsize = ( (size_t) kargs[i-1] + strlen( kargs[i-1] ) + 1 )
			  - (size_t) param;
		if ( argsize > 0 )
		{
			args = ffs_alloc ( proc->stack_pool, argsize );
			arg = (void *) args + (i + 1) * sizeof (void *);
			kargs = param;
			i = 0;
			do {
				karg = kargs[i];
				strcpy ( arg, karg );
				args[i++] = K2U_GET_ADR ( arg, proc );
				arg += strlen ( arg ) + 1;
			}
			while ( kargs[i] );
			args[i] = NULL;
			args = K2U_GET_ADR ( args, proc );
		}

		kfree ( param );
	}
	kthr = k_create_thread ( proc->pi->init, args, NULL, prio, NULL, 0, 1,
				proc );

	list_append ( &procs, proc, &proc->all );

	k_schedule_threads ();

	return kthr;
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
 * \param proc Process descriptor thread belongs to
 * \returns Pointer to descriptor of created kernel thread
 */
kthread_t *k_create_thread ( void *start_func, void *param, void *exit_func,
			     int prio, void *stack, size_t stack_size, int run,
			     kprocess_t *proc )
{
	kthread_t *kthr;

	/* if stack is not defined */
	if ( proc && proc->stack_pool && ( !stack || !stack_size ) )
	{
		stack_size = proc->pi->thread_stack;
		stack = ffs_alloc ( proc->stack_pool, stack_size );
	}
	else if ( !stack || !stack_size )
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
				     exit_func, stack, stack_size, proc );
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
	kthr->proc = proc;
	kthr->proc->thr_count++;
	kthr->private_storage = NULL;

#ifdef	MESSAGES
	k_thr_msg_init ( &kthr->msg );
#endif
	list_append ( &all_threads, kthr, &kthr->al );

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

	/* return to 'active_thread' */
	arch_select_thread ( &active_thread->context );
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
#include <api/syscall.h>

/*! Idle thread starting (and only) function */
static void idle_thread ( void *param )
{
	while (1)
		syscall ( SUSPEND, NULL );
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
int sys__create_thread ( void *p )
{
	void *func;
	void *param;
	int prio;
	thread_t *thr_desc;

	kthread_t *kthr;

	func = *( (void **) p ); p += sizeof (void *);

	param = *( (void **) p ); p += sizeof (void *);

	prio = *( (int *) p ); p += sizeof (int);

	kthr = k_create_thread (func, param, active_thread->proc->pi->exit, prio,
				NULL, 0, 1, active_thread->proc );

	ASSERT_ERRNO_AND_EXIT ( kthr, E_NO_MEMORY );

	thr_desc = *( (void **) p );
	if ( thr_desc )
	{
		thr_desc = U2K_GET_ADR ( thr_desc, active_thread->proc );
		thr_desc->thread = kthr;
		thr_desc->thr_id = kthr->id;
	}

	SET_ERRNO ( SUCCESS );

	k_schedule_threads ();

	RETURN ( SUCCESS );
}

/*! Internal function form removing (freeing) thread descriptor */
static void k_remove_thread_descriptor ( kthread_t *kthr )
{
	k_free_unique_id ( kthr->id );
	kthr->id = 0;

	list_remove ( &all_threads, FIRST, &kthr->al );

	kfree ( kthr );
}

/*!
 * End current thread (exit from it)
 * \param status Exit status number
 */
int sys__thread_exit ( void *p )
{
	int status;

	status = *( (int *) p );

	active_thread->state = THR_STATE_PASSIVE;
	active_thread->ref_cnt--;
	active_thread->exit_status = status;

	active_thread->proc->thr_count--;

#ifdef	MESSAGES
	k_msgq_clean ( &active_thread->msg.msgq );
#endif
	/* release thread stack */
	if ( active_thread->stack )
	{
		if ( active_thread->proc->m.start ) /* user level thread */
			ffs_free ( active_thread->proc->stack_pool,
				   active_thread->stack );
		else /* kernel level thread */
			kfree ( active_thread->stack );
	}

	k_delete_thread_private_storage ( active_thread,
					  active_thread->private_storage );

	if ( active_thread->proc->thr_count == 0 && active_thread->proc->pi )
	{
		/* last (non-kernel) thread - remove process */
		kfree ( active_thread->proc->pi );
		ASSERT ( list_remove ( &procs, FIRST, &active_thread->proc->all ) );
		kfree ( active_thread->proc );
	}

	if ( !active_thread->ref_cnt )
	{
		k_remove_thread_descriptor ( active_thread );

		active_thread = NULL;
	}
	else {
		k_release_all_threads ( &active_thread->join_queue );
	}

	k_schedule_threads ();

	return 0;
}

/*! Wait for thread termination
 * \param thread Thread descriptor (user level descriptor)
 * \param wait Wait if thread not finished (!=0) or not (0)?
 * \returns 0 if thread already gone; -1 if not finished and 'wait' not set;
 *          'thread exit status' otherwise
 */
int sys__wait_for_thread ( void *p )
{
	thread_t *thread;
	int wait;
	kthread_t *kthr;
	int ret_value = 0;

	thread = U2K_GET_ADR ( *( (void **) p ), active_thread->proc );
	p += sizeof (void *);

	wait = *( (int *) p );

	ASSERT_ERRNO_AND_EXIT ( thread && thread->thread, E_INVALID_HANDLE );

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

	return ret_value;
}

/*!
 * Cancel some other thread
 * \param thread Thread descriptor (user)
 */
int sys__cancel_thread ( void *p )
{
	thread_t *thread;
	kthread_t *kthr;
	int ret_value = -1;

	thread = U2K_GET_ADR ( *( (void **) p ), active_thread->proc );

	ASSERT_ERRNO_AND_EXIT ( thread && thread->thread, E_INVALID_HANDLE );

	kthr = thread->thread;

	if ( kthr->id != thread->thr_id )
		EXIT ( SUCCESS ); /* thread is already finished */

	/* remove thread from queue where its descriptor is */
	switch ( kthr->state )
	{
	case THR_STATE_PASSIVE:
		EXIT ( SUCCESS ); /* thread is already finished */

	case THR_STATE_READY:
	case THR_STATE_WAIT:
		SET_ERRNO ( SUCCESS );
		/* temporary move calling thread (active) to ready */
		move_to_ready ( active_thread );
		/* remove target 'thread' from its queue */
		k_threadq_remove ( kthr->queue, kthr );
		/* mark it as active and 'end' it normally */
		active_thread = kthr;
		active_thread->state = THR_STATE_ACTIVE;
		active_thread->queue = NULL;
		return sys__thread_exit ( &ret_value );

	case THR_STATE_ACTIVE:
	default:
		EXIT ( E_INVALID_HANDLE ); /* thread descriptor corrupted ! */
	}
}

/*! Return calling thread descriptor
 * \param thread Thread descriptor (user level descriptor)
 * \returns 0
 */
int sys__thread_self ( void *p )
{
	thread_t *thread;

	thread = U2K_GET_ADR ( *( (void **) p ), active_thread->proc );

	ASSERT_ERRNO_AND_EXIT ( thread, E_INVALID_HANDLE );

	thread->thread = active_thread;
	thread->thr_id = active_thread->id;

	EXIT ( SUCCESS );
}

/*!
 * Start new process
 * \param prog_name Program name (as given with module)
 * \param thr_desc Pointer to thread descriptor (user) for starting thread
 * \param param Command line arguments for starting thread (if not NULL)
 * \param prio Priority for starting thread
 */
int sys__start_program ( void *p )
{
	char *prog_name;
	void *param;
	int prio;
	thread_t *thr_desc;
	kthread_t *kthr, *cur = active_thread;
	char *arg, *karg, **args, **kargs = NULL;
	int argnum, argsize;

	prog_name = *( (void **) p ); p += sizeof (void *);
	ASSERT_ERRNO_AND_EXIT ( prog_name, E_INVALID_HANDLE );

	prog_name = U2K_GET_ADR ( prog_name, cur->proc );

	thr_desc = *( (void **) p ); p += sizeof (void *);

	param = *( (void **) p ); p += sizeof (void *);

	prio = *( (int *) p );

	if ( param ) /* copy parameters from one process space to another */
	{
		/* copy parameters to new process address space */
		/* first copy them to kernel */
		argnum = 0;
		argsize = 0;
		args = U2K_GET_ADR ( param, cur->proc );
		while ( args[argnum] )
		{
			arg = U2K_GET_ADR ( args[argnum++], cur->proc );
			argsize += strlen ( arg ) + 1;
		}
		if ( argnum > 0 )
		{
			kargs = kmalloc ( (argnum + 1) * sizeof (void *) +
					      argsize );
			karg = (void *) kargs + (argnum + 1) * sizeof (void *);
			argnum = 0;
			while ( args[argnum] )
			{
				arg = U2K_GET_ADR ( args[argnum], cur->proc );
				strcpy ( karg, arg );
				kargs[argnum++] = karg;
				karg += strlen ( karg ) + 1;
			}
			kargs[argnum] = NULL;
		}
	}

	SET_ERRNO ( SUCCESS );

	kthr = k_proc_start ( prog_name, kargs, prio );

	if ( !kthr )
		EXIT ( E_NO_MEMORY );

	if ( thr_desc ) /* save thread descriptor */
	{
		thr_desc = U2K_GET_ADR ( thr_desc, cur->proc );
		thr_desc->thread = kthr;
		thr_desc->thr_id = kthr->id;
	}

	RETURN ( SUCCESS );
}

/*!
 * Display info on threads
 */
int k_thread_info ()
{
	kthread_t *kthr;
	int i = 1;

	kprint ( "Threads info\n" );

	kprint ( "[this]\tid=%d in process at %x, size=%d\n",
		  active_thread->id, active_thread->proc->m.start,
		  active_thread->proc->m.size );

	kprint ( "\tprio=%d, state=%d, ret_val=%d\n",
		  active_thread->sched.prio, active_thread->state,
		  active_thread->exit_status );

	kthr = list_get ( &all_threads, FIRST );
	while ( kthr )
	{
		kprint ( "[%d]\tid=%d in process at %x, size=%d\n",
			  i++, kthr->id, kthr->proc->m.start, kthr->proc->m.size );

		kprint ( "\tprio=%d, state=%d, ret_val=%d\n",
			  kthr->sched.prio, kthr->state, kthr->exit_status );

		kthr = list_get_next ( &kthr->al );
	}

	return 0;
}

/*! Set and get current thread error status */
int sys__set_errno ( void *p )
{
	active_thread->errno = *( (int *) p );

	return 0;
}
int sys__get_errno ( void *p )
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
inline kprocess_t *k_get_thread_process ( kthread_t *thread )
{
	return thread->proc;
}
inline kprocess_t *k_get_active_process ()
{
	return active_thread->proc;
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

inline kthread_t *k_get_kthread ( thread_t *thr )
{
	kthread_t *kthr;

	if ( thr && (kthr = thr->thread) && thr->thr_id == kthr->id &&
		kthr->state != THR_STATE_PASSIVE )
		return kthr;
	else
		return NULL;
}
#ifdef	MESSAGES
inline kthrmsg_qs *k_get_thrmsg ( kthread_t *thread )
{
	return &thread->msg;
}
#endif

/*! Thread private storage */
inline void *k_create_thread_private_storage ( kthread_t *kthr, size_t s )
{
	return ffs_alloc ( kthr->proc->stack_pool, s );
}
inline void k_delete_thread_private_storage ( kthread_t *kthr, void *ps )
{
	if ( ps )
		ffs_free ( kthr->proc->stack_pool, ps );
}
inline void k_set_thread_private_storage ( kthread_t *kthr, void *ps )
{
	kthr->private_storage = ps;
}
inline void *k_get_thread_private_storage ( kthread_t *kthr )
{
	return kthr->private_storage;
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
