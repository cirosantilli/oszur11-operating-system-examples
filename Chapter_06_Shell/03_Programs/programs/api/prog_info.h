/* Program info */

#pragma once

#include <lib/types.h>

typedef struct _prog_info_t_
{
	/* defined in compile time */
	void *init;	/* process initialization function */
	void *entry;	/* starting user function */
	void *param;	/* parameter to starting function */

	/* (re)defined in run time */
	void *heap;
	size_t heap_size;

	void *mpool;
	void *stdin;
	void *stdout;
}
prog_info_t;

void prog_init ( void *args );

/* "programs" */
int hello_world ( char *args[] );
int timer ( char *args[] );
int keyboard ( char *args[] );
int shell ( char *argv[] );
int arguments ( char *argv[] );

#define PROGRAMS_FOR_SHELL	\
{	\
	{ hello_world, "hello", "Print 'Hello world'" }, \
	{ timer, "timer", "Timer interface demonstration: periodic timer activations." }, \
	{ keyboard, "keyboard", "Print ASCII code for each keystroke. Press '.' to end." }, \
	{ shell, "shell", "Simple command shell" }, \
	{ arguments, "args", "Print given command line arguments" }, \
	{ NULL, NULL, NULL } \
}
