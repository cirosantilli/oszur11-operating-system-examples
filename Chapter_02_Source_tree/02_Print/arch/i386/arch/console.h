/*! Console interface */

#pragma once

/*! Console interface */
int arch_console_init ();
int arch_console_clear ();
int arch_console_gotoxy ( int x, int y );
int arch_console_print_char ( int c, int attr );
