/* Program info */

#pragma once

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
