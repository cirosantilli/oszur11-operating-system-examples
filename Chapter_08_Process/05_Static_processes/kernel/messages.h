/*! Messages */

#pragma once

#include <lib/types.h>
#include <lib/list.h>

struct _kthrmsg_qs_;
typedef struct _kthrmsg_qs_ kthrmsg_qs;

#include <kernel/thread.h>

/*! message */
typedef struct _kmsg_t_
{
	list_h list;
	msg_t msg; /* message (variable length!) */
}
kmsg_t;

/*! kernel message queue */
typedef struct _kmsg_q_
{
	list_t msgs;	/* messages in queue (kmsg_t) */
	uint min_prio;
	/* minimal required priority of message - if less, message is dropped! */

	kthread_q thrq;
}
kmsg_q;

/*! kernel message queue (global) */
typedef struct _kgmsg_q_
{
	kmsg_q mq;

	uint id;	/* queue unique identifier */
	list_h all;	/* all global queues are in single list */
}
kgmsg_q;

/*! Message expansion for thread descriptor */
struct _kthrmsg_qs_
{
	kmsg_q msgq;

	uint sig_prio;	/* signal must have type >= min_prio to be accepted */
	void (*signal_handler) ( msg_t *msg );
};

/* flags */
#define KMSGS_ON	1
#define KSIGNALS_ON	2


void k_thr_msg_init ( kthrmsg_qs *thrmsg );
int sys__thread_msg_set ( void *p );
int sys__create_msg_queue ( void *p );
int sys__delete_msg_queue ( void *p );
int sys__msg_post ( void *p );
int sys__msg_recv ( void *p );

void k_msgq_clean ( kmsg_q *kmsgq );
