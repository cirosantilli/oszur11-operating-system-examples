/* Program info */

#include "prog_info.h"
#include <api/malloc.h>

extern int PROG_START_FUNC ( char *args[] );

prog_info_t pi =
{
	.init = 	prog_init,
	.entry =	PROG_START_FUNC,
	.param =	NULL,

	.heap =		NULL,
	.heap_size =	0,

	.mpool =	NULL,
	.stdin =	NULL,
	.stdout =	NULL
};


/*! Initialize threads environment */
void prog_init ( void *args )
{
	/* initialize dynamic memory */
	pi.mpool = mem_init ( pi.heap, pi.heap_size );

	/* call starting function */
	( (void (*) ( void * ) ) pi.entry ) ( args );
}
