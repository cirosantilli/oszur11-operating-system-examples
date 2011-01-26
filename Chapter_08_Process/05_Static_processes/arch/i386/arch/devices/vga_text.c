/*! Print on console using video memory */
#ifdef VGA_TEXT /* using this devices? */

#include <arch/io.h>
#include <devices/devices.h>
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

static int vga_text_init ();
static int vga_text_clear ();
static int vga_text_gotoxy ( int x, int y );
static int vga_text_print_char ( int c, int attr );

/*!
 * Init console
 */
static int vga_text_init ( void *x )
{
	video = (unsigned char *) VIDEO;
	xpos = ypos = 0;

	return vga_text_clear ();
}

/*!
 * Clear console
 */
static int vga_text_clear ()
{
	int i;

	for ( i = 0; i < COLS * ROWS * 2; i++ )
		*( video + i ) = 0;

	return vga_text_gotoxy ( 0, 0 );
}

/*!
 * Move cursor to specified location
 * \param x Row where to put cursor
 * \param y Column where to put cursor
 */
static int vga_text_gotoxy ( int x, int y )
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
static int vga_text_print_char ( int c, int attr )
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

	vga_text_gotoxy ( xpos, ypos );

	return 0;
}

static int vga_text_send ( void *data, size_t size, uint flags, device_t *dev )
{
	int *p, retval;

	p = (int *) data;

	switch ( flags )
	{
		case PRINTCHAR:
			retval = vga_text_print_char ( p[0], p[1] );
			break;

		case CLEAR:
			retval = vga_text_clear ();
			break;

		case GOTOXY:
			retval = vga_text_gotoxy ( p[0], p[1] );
			break;

		default:
			retval = -1;
			break;
	}

	return retval;
}

/*! vga_text as kdevice_t -----------------------------------------------------*/
device_t vga_text_dev = (device_t)
{
	.dev_name = "VGA_TXT",
	.irq_num = 	-1,
	.irq_handler =	NULL,

	.init =		(void *) vga_text_init,
	.destroy =	NULL,
	.send =		vga_text_send,
	.recv =		NULL,

	.flags = 	DEV_TYPE_SHARED,
	.params = 	(void *) &vga_text_dev
};


#endif /* VGA_TEXT */
