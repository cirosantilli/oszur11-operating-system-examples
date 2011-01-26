/*! Devices - common interface */

#pragma once

#include <lib/list.h>
#include <lib/types.h>
#include <devices/devices.h>
#include <kernel/thread.h>

typedef struct _kdevice_t_
{
	device_t dev;

	int open;

	/* locking device */
	int locked; /* is locked */
	kthread_q thrq; /* locked threads wait in queue */

	list_h list; /* all devices are in list */
}
kdevice_t;

/* interface */
int k_devices_init ();
kdevice_t *k_device_add ( device_t *kdev );
int k_device_init ( kdevice_t *kdev, int flags, void *params, void *callback );
int k_device_remove ( kdevice_t *kdev );

kdevice_t *k_device_open ( char *name );
void k_device_close ( kdevice_t *kdev );

int k_device_send ( void *data, size_t size, int flags, kdevice_t *kdev );
int k_device_recv ( void *data, size_t size, int flags, kdevice_t *kdev );

int k_device_lock ( kdevice_t *dev, int wait );
int k_device_unlock ( kdevice_t *dev );

int sys__device_send ( void *p );
int sys__device_recv ( void *p );

int sys__device_open ( void *p );
int sys__device_close ( void *p );

int sys__device_lock ( void *p );
int sys__device_unlock ( void *p );

/* include device headers */
#include <arch/devices.h>
