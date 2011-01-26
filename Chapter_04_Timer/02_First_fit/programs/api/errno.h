/*! Error numbers, macros, ... */

#pragma once

#include <lib/errno.h>

#ifdef DEBUG

/* assert and return */
#define ASSERT_ERRNO_AND_RETURN(expr, errnum)			\
do {								\
	if ( !( expr ) )					\
	{							\
		print("%s:%d::ASSERT\n", __FILE__, __LINE__ );	\
		return -errnum;					\
	 }							\
} while(0)

#else

#define ASSERT_ERRNO_AND_RETURN(expr, errnum)

#endif
