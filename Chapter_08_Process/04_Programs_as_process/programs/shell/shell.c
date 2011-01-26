/*! simple shell interpreter */

#include <api/stdio.h>
#include <lib/string.h>
#include <api/time.h>
#include <api/syscall.h>
#include <lib/types.h>

#include PROGRAMS

char PROG_HELP[] = "Simple command shell";

typedef struct _cmd_t_
{
	int (*func) ( char *argv[] );
	char *name;
	char *descr;
}
cmd_t;

#define MAXCMDLEN	72
#define MAXARGS		10
#define INFO_SIZE	1000

static int help ();
static int clear ();
static int sysinfo ( char *args[] );

static cmd_t sh_cmd[] =
{
	{ help, "help", "help - list available commands" },
	{ clear, "clear", "clear - clear screen" },
	{ sysinfo, "sysinfo", "system information; usage: sysinfo [options]" },
	{ NULL, "" }
};

static cmd_t prog[] = PROGRAMS_FOR_SHELL;


int shell ( char *args[] )
{
	char cmd[MAXCMDLEN + 1];
	int i, key;
	time_t t;
	int argnum;
	char *argval[MAXARGS + 1];

	print ( "\n*** Simple shell interpreter ***\n\n" );
	help ();

	t.sec = 0;
	t.nsec = 100000000; /* 100 ms */

	while (1)
	{
		new_cmd:
		print ( "\n> " );

		i = 0;
		memset ( cmd, 0, MAXCMDLEN );

		/* get command - get chars until new line is received */
		while ( i < MAXCMDLEN )
		{
			key = get_char ();

			if ( !key )
			{
				delay ( &t );
				continue;
			}

			if ( key == '\n' )
			{
				if ( i > 0 )
					break;
				else
					goto new_cmd;
			}

			switch ( key )
			{
			case '\b':
				if ( i > 0 )
				{
					cmd[--i] = 0;
					print ( "%c", key );
				}
				break;

			default:
				print ( "%c", key );
				cmd[i++] = key;
				break;
			}
		}
		print ( "\n" );

		/* parse command line */
		argnum = 0;
		for(i = 0; i < MAXCMDLEN && cmd[i]!=0 && argnum < MAXARGS; i++)
		{
			if ( cmd[i] == ' ' || cmd[i] == '\t')
				continue;

			argval[argnum++] = &cmd[i];
			while ( cmd[i] && cmd[i] != ' ' && cmd[i] != '\t'
				&& i < MAXCMDLEN )
				i++;

			cmd[i] = 0;
		}
		argval[argnum] = NULL;

		if ( strcmp ( argval[0], "quit" ) == 0 ||
			strcmp ( argval[0], "exit" ) == 0 )
			break;

		/* match command to shell command */
		for ( i = 0; sh_cmd[i].func != NULL; i++ )
		{
			if ( strcmp ( argval[0], sh_cmd[i].name ) == 0 )
			{
				if ( sh_cmd[i].func ( argval ) )
					print ( "\nProgram returned error!\n" );

				goto new_cmd;
			}
		}

		/* match command to program */
		for ( i = 0; prog[i].func != NULL; i++ )
		{
			if ( strcmp ( argval[0], prog[i].name ) == 0 )
			{
				if ( prog[i].func ( argval ) )
					print ( "\nProgram returned error!\n" );

				goto new_cmd;
			}
		}

		/* not program kernel or shell knows about it - report error! */
		print ( "Invalid command!" );
	}

	print ( "Exiting from shell\n" );

	return 0;
}

static int help ()
{
	int i;

	print ( "Shell commands: " );
	for ( i = 0; sh_cmd[i].func != NULL; i++ )
		print ( "%s ", sh_cmd[i].name );
	print ( "quit/exit\n" );

	print ( "Programs: " );
	for ( i = 0; prog[i].func != NULL; i++ )
		print ( "%s ", prog[i].name );
	print ( "\n" );

	return 0;
}

static int clear ()
{
	return clear_screen ();
}

static int sysinfo ( char *args[] )
{
	char info[INFO_SIZE];

	syscall ( SYSINFO, &info, INFO_SIZE, args );

	print ( "%s\n", info );

	return 0;
}
