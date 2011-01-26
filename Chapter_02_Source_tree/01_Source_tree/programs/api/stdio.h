/*! Printing on stdout, reading from stdin */

#pragma once

#include <kernel/kprint.h>

#define print_word	kprint_word

/*
or:

int print_word ( char *word )
{
	return kprint_word ( word );
}

in stdio.c

*/