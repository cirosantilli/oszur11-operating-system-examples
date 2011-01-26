/*! Semaphores (thread synchronization) */

#pragma once

#include <lib/types.h>

int sem_init ( sem_t *sem, int initial_value );
int sem_destroy ( sem_t *sem );

int sem_post ( sem_t *sem );
int sem_wait ( sem_t *sem );
