/*! Thread management functions */

#pragma once

#include <lib/types.h>

int monitor_init ( monitor_t *monitor );
int monitor_destroy ( monitor_t *monitor );
int monitor_queue_init ( monitor_q *queue );
int monitor_queue_destroy ( monitor_q *queue );

int monitor_lock ( monitor_t *monitor );
int monitor_unlock ( monitor_t *monitor );

int monitor_wait ( monitor_t *monitor, monitor_q *queue );
int monitor_signal ( monitor_q *queue );
int monitor_broadcast ( monitor_q *queue );
