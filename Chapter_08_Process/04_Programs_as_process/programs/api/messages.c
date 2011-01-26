/*! Messages and signals (threads) */

#ifdef MESSAGES

#include "messages.h"
#include <api/syscall.h>
#include <api/stdio.h>
#include <api/errno.h>

int thread_msg_set ( uint min_msg_prio, int min_sig_prio, void *sig_handler )
{
	return syscall ( THREAD_MESG_SET, min_msg_prio, min_sig_prio,
			 sig_handler );
}

int create_message_queue ( msg_q *queue, uint min_prio )
{
	ASSERT_ERRNO_AND_RETURN ( queue, E_INVALID_ARGUMENT );
	return syscall ( CREATE_MESG_Q, queue, min_prio );
}

int delete_message_queue ( msg_q *queue )
{
	ASSERT_ERRNO_AND_RETURN ( queue, E_INVALID_ARGUMENT );
	return syscall ( DELETE_MESG_Q, queue );
}

int send_message ( int dest_type, void *dest, msg_t *msg, uint flags )
{
	int retval;

	ASSERT_ERRNO_AND_RETURN ( dest, E_INVALID_ARGUMENT );

	do {
		retval = syscall ( SEND_MESG, dest_type, dest, msg, flags );
	}
	while ( retval == -E_RETRY && ( flags & IPC_WAIT ) );

	return retval;
}

int receive_message ( int src_type, void *src, msg_t *msg, int type,
		      size_t size, uint flags )
{
	int retval;

	ASSERT_ERRNO_AND_RETURN ( src, E_INVALID_ARGUMENT );

	do {
		retval = syscall ( RECV_MESG, src_type, src, msg, type, size,
				   flags );
	}
	while ( retval == -E_RETRY && ( flags & IPC_WAIT ) );

	return retval;
}

/*
TODO - limit number and/or size of messages in particular queue:
	* use system wide 'hard' limit (compiled or defined in kernel)
	* use local per-queue limit (defined with create_message_queue)
     - save all messages, even ones with lesser priority, even signals:
	* use them when min. priority is lowered
*/

#endif
