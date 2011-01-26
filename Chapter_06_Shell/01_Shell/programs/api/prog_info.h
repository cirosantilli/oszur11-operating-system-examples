/* Program info */

#pragma once

/* "programs" */
int hello_world ();
int timer ();
int keyboard ();
int shell ();

#define PROGRAMS_FOR_SHELL	\
{	\
	{ hello_world, "hello", "Print 'Hello world'" }, \
	{ timer, "timer", "Timer interface demonstration: periodic timer activations." }, \
	{ keyboard, "keyboard", "Print ASCII code for each keystroke. Press '.' to end." }, \
	{ shell, "shell", "Simple command shell" }, \
	{ NULL, NULL, NULL } \
}
