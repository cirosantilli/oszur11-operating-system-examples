/*! Simple program prepared for little debugging */

#include <api/stdio.h>

static int inc ( int n )
{
	n++;

	return n;
}

int debug ()
{
	int a, b, c;

	a = 1;

	b = a + 1;

	c = inc ( a ) + inc ( b );

	a += b + c;
	b += a + c;
	c += a + b;

	print ( "a=%d, b=%d, c=%d\n", a, b, c );

	return 0;
}
