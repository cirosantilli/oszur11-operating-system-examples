/*! Basic types, machine dependent */

#pragma once

typedef	char 			int8;
typedef	unsigned char 		uint8;
typedef	short int		int16;
typedef	unsigned short int	uint16;
typedef	int 			int32;
typedef	unsigned int 		uint32;
typedef	unsigned int 		uint;

typedef	long long int		int64;
typedef	unsigned long long int	uint64;

/* integer type with same width as pointers */
typedef unsigned int 		aint; /* sizeof(aint) == sizeof(void *) */

/* processor's 'int' size */
#define __WORD_SIZE		32
typedef unsigned int		word_t;
typedef int			sword_t; /* "signed" word */
