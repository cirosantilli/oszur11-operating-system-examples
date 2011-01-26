/*! Error numbers, macros, ... */

#pragma once

#include <lib/errno.h>
#include <api/syscall.h>

static inline int set_errno ( int error_number )
{
	return syscall ( SET_ERRNO, error_number );
}
static inline int get_errno ()
{
	return syscall ( GET_ERRNO );
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

#ifndef ASSERT

#include <api/thread.h>

#define ASSERT(expr)						\
do if ( !( expr ) )						\
{								\
	print ( "[BUG:%s:%d]\n", __FILE__, __LINE__);		\
	thread_exit ( -1 );					\
} while(0)
#endif /* ASSERT */

#ifndef LOG
#define LOG(LEVEL, format, ...)	\
print ( "[" #LEVEL ":%s:%d]" format, __FILE__, __LINE__, ##__VA_ARGS__)
#endif /* LOG */

#else

#define ASSERT_ERRNO_AND_RETURN(expr, errnum)
#define ASSERT(expr)
#define LOG(LEVEL, format, ...)

#endif
