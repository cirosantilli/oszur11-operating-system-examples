/*! Devices - common interface */

#pragma once

#include <lib/types.h>

/*! syscall wrappers -------------------------------------------------------- */

int sys__device_send ( void *data, size_t size, int flags, void *dev );
int sys__device_recv ( void *data, size_t size, int flags, void *dev );

#ifndef _KERNEL_

typedef void kdevice_t;

#else /* _KERNEL_ */

#include <lib/list.h>
#include <devices/devices.h>

typedef struct _kdevice_t_
{
	device_t dev;

	int open;

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

int sys__device_open ( char *dev_name, void **dev );
int sys__device_close ( void *dev );

/* include device headers */
#include <arch/devices.h>

#endif /* _KERNEL_ */