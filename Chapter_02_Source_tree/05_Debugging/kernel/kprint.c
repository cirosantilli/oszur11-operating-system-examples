/*! Formated printing on console (using 'console_t' interface) */
#define _KERNEL_

#include "kprint.h"
#include <arch/console.h>
#include <lib/print.h>

extern console_t *k_stdout; /* defined in startup.c */

/*! Formated output to console (lightweight version of printf) */
int kprint ( char *format, ... )
{
	return __print ( k_stdout, KERNEL_FONT, &format );
}
