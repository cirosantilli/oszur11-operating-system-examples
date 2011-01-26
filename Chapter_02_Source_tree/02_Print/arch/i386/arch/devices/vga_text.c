/*! Print on console using video memory */
#ifdef VGA_TEXT /* using this devices? */

#include <arch/console.h>
#include <arch/io.h>
#include <lib/types.h>
#include <lib/string.h>

#define VIDEO		0x000B8000 /* video memory address */
#define COLS		80 /* number of characters in a column */
#define ROWS		24 /* number of characters in a row */

/* cursor position */
static int xpos;
static int ypos;

/* starting address of video memory */
static unsigned char *video;

/* font color */
static int color[3] = {
	7, /* 'normal' characters - white on black background */
	4, /* 'kernel' font - red */
	2  /* 'program' font - green */
};

/*! Init console */
int arch_console_init ()
{
	video = (unsigned char *) VIDEO;
	xpos = ypos = 0;

	return arch_console_clear ();
}

/*! Clear console */
int arch_console_clear ()
{
	int i;

	for ( i = 0; i < COLS * ROWS * 2; i++ )
		*( video + i ) = 0;

	return arch_console_gotoxy ( 0, 0 );
}

/*!
 * Move cursor to specified location
 * \param x Row where to put cursor
 * \param y Column where to put cursor
 */
int arch_console_gotoxy ( int x, int y )
{
	unsigned short int t;

	xpos = x;
	ypos = y;
	t = ypos * 80 + xpos;

	outb ( 0x3D4, 14 );
	outb ( 0x3D5, t >> 8 );
	outb ( 0x3D4, 15 );
	outb ( 0x3D5, t & 0xFF );

	return 0;
}

/*!
 * Print single character on console on current cursor position
 * \param c Character to print
 */
int arch_console_print_char ( int c, int attr )
{
	switch ( c ) {
	case '\t': /* tabulator */
		xpos = ( xpos / 8 + 1 ) * 8;
		break;
	case '\r': /* carriage return */
		xpos = 0;
	case '\n': /* new line */
		break;
	case '\b': /* backspace */
		if ( xpos > 0 )
		{
		xpos --;
		*( video + ( xpos + ypos * COLS ) * 2 ) = ' ' & 0x00FF;
		*( video + ( xpos + ypos * COLS ) * 2 + 1 ) = color[attr];
		}
		break;
	default:
		*(video + (xpos + ypos * COLS) * 2) = c & 0x00FF;
		*(video + (xpos + ypos * COLS) * 2 + 1) = color[attr];
		xpos++;
	}

	if ( xpos >= COLS || c == '\n' ) /* continue on new line */
	{
		xpos = 0;
		if ( ypos < ROWS )
		{
			ypos++;
		}
		else { /* scroll one line */
			memmove ( video, video + 80 * 2, 80 * 2 * 24 );
			memsetw ( video + 80 * 2 * 24,
				  ' ' | ( color[attr] << 8 ), 80 );
		}
	}

	arch_console_gotoxy ( xpos, ypos );

	return 0;
}

#endif /* VGA_TEXT */
