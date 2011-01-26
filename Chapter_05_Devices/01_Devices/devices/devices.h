/*! Device - common interface */

#pragma once

#include <lib/types.h>

#define DEV_NAME_LEN	32

#define DEV_TYPE_SHARED		1
#define DEV_TYPE_NOTSHARED	2

struct _device_t_;
typedef struct _device_t_ device_t;

struct _device_t_
{
	char dev_name[DEV_NAME_LEN]; /* device name */

	/* which IRQ is using? (if any, -1 otherwise) */
	int irq_num;

	/* interrupt handler function
	   (test if device is interrupt source and handle it if it is) */
	void (*irq_handler) ( int irq_num, void *device );

	/* callback function (to kernel) - when event require kernel action */
	int (*callback) ( int irq_num, void *device );

	/* device interface */
	int (*init) ( uint flags, void *params, device_t *dev );
	int (*destroy) ( uint flags, void *params, device_t *dev );
	int (*send) ( void *data, size_t size, uint flags, device_t *dev );
	int (*recv) ( void *data, size_t size, uint flags, device_t *dev );

	/* various flags and parameters specific to device */
	int flags;
	void *params;
};