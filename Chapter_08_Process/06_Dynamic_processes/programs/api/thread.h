/*! Threads */

#pragma once

#include <lib/types.h>

int create_thread ( void *start_func, void *param, int prio, thread_t *handle);
void thread_exit ( int status );// __attribute__(( noinline ));
int wait_for_thread ( void *thread, int wait );
int cancel_thread ( void *thread );
int thread_self ( thread_t *thr );

int start_program ( char *prog_name, thread_t *handle, void *param, int prio );