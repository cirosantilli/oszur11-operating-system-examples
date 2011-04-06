/*! Thread management */

#pragma once

#include <lib/types.h>

/*! interface for threads --------------------------------------------------- */
int sys__create_thread ( void *func, void *param, int prio, thread_t *thread );
int sys__thread_exit ( int status );
int sys__wait_for_thread ( thread_t *thread, int wait );
int sys__cancel_thread (thread_t *thread );
int sys__thread_self ( thread_t *thread );

int sys__set_errno ( int errno );
int sys__get_errno ();

#ifdef _KERNEL_ /* for use inside 'kernel/ *' files */

#include <lib/types.h>
#include <lib/list.h>

/*! Thread queue */
typedef struct _kthread_q_
{
	list_t q;		/* queue implementation in list.h/list.c */
	/* uint flags; */	/* various flags, e.g. sort order */
}
kthread_q;

/* only 'kernel/thread.c' 'knows' thread descriptor details */
#ifndef _K_THREAD_C_
typedef void *kthread_t;
#else /* only for 'kernel/thread.c' */
struct _kthread_t_;
typedef struct _kthread_t_ kthread_t;
#endif /* _K_THREAD_C_ */

#include <kernel/memory.h>
#include <kernel/messages.h>

/*! Interface for kernel ---------------------------------------------------- */
void k_threads_init ();
void k_schedule_threads ();
kthread_t *k_create_thread ( void *start_func, void *param, void *exit_func,
	int prio, void *stack, size_t stack_size, int run );

/*! Get-ers and Set-ers */
extern inline void *k_get_active_thread ();
extern inline void *k_get_thread_context ( kthread_t *thread );
extern inline void *k_get_active_thread_context ();
extern inline int k_get_thread_prio ( kthread_t *thread );
extern inline int k_get_active_thread_prio ();

extern inline void *k_create_thread_private_storage (kthread_t *kthr, size_t s);
extern inline void k_delete_thread_private_storage ( kthread_t *kthr, void *ps );
extern inline void k_set_thread_private_storage ( kthread_t *kthr, void *ps );
extern inline void *k_get_thread_private_storage ( kthread_t *kthr );

#ifdef	MESSAGES
extern inline kthrmsg_qs *k_get_thrmsg ( kthread_t *thread );
#endif

extern inline kthread_t *k_get_kthread ( thread_t *thr );

/*! Thread queue manipulation */
extern inline void k_threadq_init ( kthread_q *q );
extern inline void k_threadq_append ( kthread_q *q, kthread_t *kthr );
extern inline kthread_t *k_threadq_remove ( kthread_q *q, kthread_t *kthr );
extern inline kthread_t *k_threadq_get ( kthread_q *q );
extern inline kthread_t *k_threadq_get_next ( kthread_t *kthr );

void k_enqueue_thread ( kthread_t *kthr, kthread_q *q_id );
int k_release_thread ( kthread_q *q_id );
int k_release_all_threads ( kthread_q *q_id );

extern inline void k_set_thread_qdata ( kthread_t *kthr, void *qdata );
extern inline void *k_get_thread_qdata ( kthread_t *kthr );

/*! errno */
extern inline void k_set_errno ( int error_number );
extern inline void k_set_thread_errno ( kthread_t *kthr, int error_number );
extern inline int k_get_errno ();
extern inline int k_get_thread_errno ( kthread_t *kthr );

int k_thread_info ();

#ifdef _K_THREAD_C_ /* rest of the file is only for kernel/thread.c */

/* Scheduling parameters */
typedef struct _ksched_t_
{
	int prio; /* for now just priority (secondary criteria is FIFO) */
	/* uint flags; */
}
ksched_t;

#include <arch/context.h>

/*! Thread descriptor */
struct _kthread_t_
{
	context_t context;	/* storage for thread context */

	uint id;		/* thread id */

	int state;		/* thread state */

	list_h ql;		/* list element for "thread state" list */

	ksched_t sched;		/* scheduling parameters */

	kthread_q *queue;	/* in witch queue (if not active) */
	void *qdata;		/* temporary storage for data while waiting */

	kthread_q join_queue;	/* queue for threads waiting for this to end */

	void *stack;		/* stack address and size (for deallocation) */
	uint stack_size;

	void *private_storage;	/* thread private storage */

#ifdef	MESSAGES
	kthrmsg_qs msg;
#endif

	list_h all;		/* list element for list of all threads */

	int exit_status;	/* status with which thread exited */

	int errno;		/* exit status of last function call */

	int ref_cnt;		/* can we free this descriptor? */
};

/*! Thread states */
enum {
	THR_STATE_ACTIVE = 1,
	THR_STATE_READY,
	THR_STATE_WAIT,
	THR_STATE_PASSIVE
};

/* "ready" queue manipulation */
static void init_ready_list ();
static int get_top_ready ();
static void set_got_ready ( int index );
static void clear_got_ready ( int index );
static void move_to_ready ( kthread_t *kthr );

/* idle thread */
static void idle_thread ( void *param );

#endif	/* _K_THREAD_C_ */
#endif	/* _KERNEL_ */
