/*! Printing on stdout, reading from stdin */

#pragma once

extern inline int clear_screen ();
extern inline int goto_xy ( int x, int y );
extern inline int print_char ( int c, int attr );
int print ( char *format, ... );
