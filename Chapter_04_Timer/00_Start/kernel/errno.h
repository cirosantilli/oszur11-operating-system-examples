/*! Error numbers, macros, ... */

#pragma once

#include <lib/errno.h>

/*! return from function (syscall) with code (success or error number) */
#define EXIT(ENUM)	return (-ENUM)


/*! debugging 'support' */
#ifdef DEBUG

#include <arch/processor.h>
#include <kernel/kprint.h>

/*! Debugging outputs (includes files and line numbers!) */
#define LOG(LEVEL, format, ...)	\
kprint ( "[" #LEVEL ":%s:%d]" format "\n", __FILE__, __LINE__, ##__VA_ARGS__)

/*! Critical error - print it and stop */
#define ASSERT(expr)						\
do if ( !( expr ) )						\
{								\
	kprint ( "[BUG:%s:%d]\n", __FILE__, __LINE__);		\
	halt();							\
} while(0)

/*! Error - print it and immediately return from function with error number as
    return value */
#define ASSERT_ERRNO_AND_EXIT(expr, errnum)	\
do {	if ( !( expr ) )			\
	{					\
		LOG ( ASSERT, "" );		\
		EXIT ( errnum );		\
	}					\
} while(0)

#else /* !DEBUG */

#define ASSERT(expr)
#define ASSERT_ERRNO_AND_EXIT(expr, errnum)
#define LOG(LEVEL, format, ...)

#endif /* DEBUG */
