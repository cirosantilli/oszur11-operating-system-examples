/*! Simple semaphore */

#pragma once

#include <lib/types.h>
#include <kernel/thread.h>

typedef struct _ksem_t_
{
	int sem_value;

	kthread_q queue;
}
ksem_t;

int sys__sem_init ( void *p );
int sys__sem_destroy ( void *p );

int sys__sem_post ( void *p );
int sys__sem_wait ( void *p );

