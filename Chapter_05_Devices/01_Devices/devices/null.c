/*! null device (test only, not used) */

#include <devices/devices.h>

static int do_nothing ()
{
	return 0;
}

device_t dev_null = (device_t)
{
	.dev_name = "dev_null",

	.irq_num = 	-1,
	.irq_handler =	NULL,

	.init =		NULL,
	.destroy =	NULL,
	.send =		do_nothing,
	.recv =		do_nothing,

	.flags = 	DEV_TYPE_SHARED,
	.params = 	NULL,
};
