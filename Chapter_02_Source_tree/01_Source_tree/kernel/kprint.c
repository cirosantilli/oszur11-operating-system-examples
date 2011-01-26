/*! Print "Hello world" (user arch layer to do it) */

#include "kprint.h"
#include <arch/print.h>

int kprint_word ( char *word )
{
	return arch_console_print_word ( word );
}
