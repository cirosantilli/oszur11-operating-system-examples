/*! Error numbers, macros, ... */

#pragma once

#include <lib/errno.h>
#include <api/syscalls.h>

static inline int set_errno ( int error_number )
{
	return sys__set_errno ( error_number );
}
static inline int get_errno ()
{
	return sys__get_errno ();
}

#ifdef DEBUG

/* assert and return */
#define ASSERT_ERRNO_AND_RETURN(expr, errnum)			\
do {								\
	if ( !( expr ) )					\
	{							\
		print("%s:%d::ASSERT\n", __FILE__, __LINE__ );	\
		set_errno (-errnum);				\
		return -errnum;					\
	 }							\
} while(0)

#else

#define ASSERT_ERRNO_AND_RETURN(expr, errnum)

#endif
