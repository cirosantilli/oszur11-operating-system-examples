/*! Simple semaphore */

#pragma once

#include <lib/types.h>

int sys__sem_init ( sem_t *sem, int initial_value );
int sys__sem_destroy ( sem_t *sem );

int sys__sem_post ( sem_t *sem );
int sys__sem_wait ( sem_t *sem );

#ifdef _KERNEL_

#include <kernel/thread.h>

typedef struct _ksem_t_
{
	int sem_value;

	kthread_q queue;
}
ksem_t;

#endif
