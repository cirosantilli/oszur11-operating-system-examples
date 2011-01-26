/*! Basic data types and constants */

#pragma once

#include <arch/types.h>

#if __WORD_SIZE >= 32
typedef word_t	size_t;
typedef sword_t	ssize_t;
#else /* size_t must be 32 bits or more */
typedef uint32 size_t;
typedef uint32 ssize_t;
#endif

#define NULL		((void *) 0)

#define FALSE		0
#define TRUE		1

