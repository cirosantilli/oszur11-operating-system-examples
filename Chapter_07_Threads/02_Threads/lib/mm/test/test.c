/*! standalone memory allocator test (test for errors) */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <time.h>

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
struct timespec { long tv_sec; long tv_nsec; };
#endif /* CLOCK_REALTIME */

#define ERROR(format, ...)	\
	printf ( "[ERROR:%s:%d]" format, __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG(level, format, ...)	\
printf ( "[" #level ":%s:%d]" format "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define ASSERT(expr)					\
do if ( !( expr ) )					\
{							\
	printf ( "[BUG:%s:%d]\n", __FILE__, __LINE__);	\
	exit (1);					\
} while(0)

#undef NULL
#if defined ( FIRST_FIT )

#include "../ff_simple.c"

#define	MEM_INIT(ADDR, SIZE)		ffs_init ( ADDR, SIZE )
#define MEM_ALLOC(MP, SIZE)		ffs_alloc ( MP, SIZE )
#define MEM_FREE(MP, ADDR)		ffs_free ( MP, ADDR )

#elif defined ( GMA )

#include "../gma.c"

#define	MEM_INIT(ADDR, SIZE)		gma_init ( ADDR, SIZE, 32, 0 )
#define MEM_ALLOC(MP, SIZE)		gma_alloc ( MP, SIZE )
#define MEM_FREE(MP, ADDR)		gma_free ( MP, ADDR )

#endif

/* testing */
int main ()
{
	int pool_size = 1234567;
	int max_block_size = 1512;
	int init_requests = 1500, requests = 100000;
	int i, j, k, used, fail;
	size_t inuse = 0;
	struct req
	{
		void *ptr;
		unsigned int size;
	}
	m[requests];
	void *pool, *mpool;
	struct timespec t1, t2;

	if ( ( pool = malloc ( pool_size ) ) == NULL )
	{
		printf ( "Malloc return NULL\n" );
		return 1;
	}

	memset ( pool, 0, pool_size );

	for ( j = 0; j < requests; j++)
	{
		m[j].ptr = NULL;
		m[j].size = 0;
	}

	mpool = MEM_INIT ( pool, pool_size );

	used = 0;
	fail = 0;

	/* initial allocations */
	for ( j = 0; j < init_requests; j++)
	{
		m[j].size = lrand48() % max_block_size + 4;
		clock_gettime(CLOCK_REALTIME, &t1);
		m[j].ptr = MEM_ALLOC ( mpool, m[j].size );
		clock_gettime(CLOCK_REALTIME, &t2);

		if ( m[j].ptr != NULL )
		{
//			printf("[%d] alloc=%p\t[%u]\n", j, m[j].ptr, m[j].size);

			memset ( m[j].ptr, 5, m[j].size );
			used++;
			inuse += m[j].size;

			printf ( "%u %u %ld\n", (unsigned int) m[j].ptr, m[j].size,
				 (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec - t1.tv_nsec );
		}
		else {
			fail++;
			printf("[%d] alloc=%p\t[%u]\n", j, m[j].ptr, m[j].size);
			printf ( "FAIL(%d)\n", fail );

			break;
		}
	}

	printf ( "Start of tests (j=%d, fail=%d, inuse=%d)!\n", j, fail, inuse );

	fail = 0;

	for ( i = 0; i < requests; i++ )
	{
		if ( lrand48() & 1 )
		{
			/* alloc */
			for ( j = 0; j < requests && m[j].ptr != NULL; j++)
				;

			if ( j >= requests )
			{
				printf ( "No free element in m[]!\n" );
				break;
			}

			m[j].size = lrand48() % (max_block_size) + 4;
			//printf ( "before alloc\n" );

			clock_gettime(CLOCK_REALTIME, &t1);
			m[j].ptr = MEM_ALLOC ( mpool, m[j].size );
			clock_gettime(CLOCK_REALTIME, &t2);

			//printf ( "after alloc (%p)\n", m[j].ptr );

			if ( m[j].ptr != NULL )
			{
				//printf ( "[%d] alloc=%p\t[%u]\n", i, m[j].ptr,
				//	 m[j].size );
				memset ( m[j].ptr, 3, m[j].size );
				used++;
				inuse += m[j].size;

				printf ( "%u %ud %ld\n", (unsigned int) m[j].ptr, m[j].size,
					 (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec - t1.tv_nsec );
			}
			else {
				fail++;
//				printf ( "FAIL(%d)\n", fail );
				if ( fail == 1 )
					printf ( "\tFirst fail (i=%d)!\n", i );
			}
		}
		else {
			/* free */
			while ( used > 0 )
			{
				k = lrand48() % requests;
				if ( m[k].ptr != NULL )
				{
					//printf ( "[%d] free =%p\t[%u]\n",
					//	 i, m[k].ptr, m[k].size );

					MEM_FREE ( mpool, m[k].ptr );
					//printf ( "free done\n" );

					m[k].ptr = NULL;

					used--;
					inuse -= m[j].size;

					break;
				}
			}
		}
	}

	//printf ( "\n" );

	printf ( "End of tests (i=%d, fail=%d, inuse=%d)!\n", i, fail, inuse );

	return 0;
}
