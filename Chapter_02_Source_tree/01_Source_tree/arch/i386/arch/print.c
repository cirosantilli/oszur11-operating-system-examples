/*! Print on console using video memory */

#include <arch/print.h>
#include <lib/types.h>
#include <lib/string.h>

#define VIDEO	( (char *) 0x000B8000 ) /* video memory address */
#define COLS	80 /* number of characters in a column */
#define ROWS	24 /* number of characters in a row */
#define ATTR	7  /* font: white char on black bacground */

/*! Init console */
int arch_console_init ()
{
	/* erase screen (set blank screen) */
	memset ( VIDEO, 0, COLS * ROWS * 2 );

	return 0;
}

/*!
 * Print single word console starting from top left corner
 * \param word Word to print (word must be shorter than 80 characters)
 */
int arch_console_print_word ( char *word )
{
	char *video = VIDEO;
	int i;

	if ( word == NULL )
		return -1;

	/* erase first line */
	memset ( video, 0, COLS * 2 );

	/* print word on first line */
	for ( i = 0; word[i] != 0 && i < COLS; i++ )
	{
		*( video + i * 2 )     = word[i];
		*( video + i * 2 + 1 ) = ATTR;
	}

	return 0;
}
