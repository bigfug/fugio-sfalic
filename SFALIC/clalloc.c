#include "clalloc.h"

#ifndef CLALLOC_OFF

#include <assert.h>
#include "ceillog2.h"


/* allocated memory blocks 0 - actual address, 1-alligned */
void * clablocks[2][CLALLOC_TAB_SIZE];

/* block counter */
int nblocks=0;


void *clacalloc(size_t num, size_t size)
{
	void *pv, *ap;
	assert ( 0x1<<ceil_log_2(CACHE_LINE_BYTES) == CACHE_LINE_BYTES );

	if (nblocks==CLALLOC_TAB_SIZE)
		return NULL;

	pv=calloc(CACHE_LINE_BYTES - 1 + num * size, 1);

	if (!pv)
		return NULL;

	ap=(void *)( (char *)0 + ((((char *)pv - (char *)0) + CACHE_LINE_BYTES - 1) & ~((unsigned long)CACHE_LINE_BYTES-1)) );

	clablocks[0][nblocks]=pv;
	clablocks[1][nblocks]=ap;
	nblocks++;

	return ap;
}


void *clamalloc(size_t bytes)
{
	void *pv, *ap;
	assert ( 0x1<<ceil_log_2(CACHE_LINE_BYTES) == CACHE_LINE_BYTES );

	if (nblocks==CLALLOC_TAB_SIZE)
		return NULL;

	pv=malloc(CACHE_LINE_BYTES - 1 + bytes);

	if (!pv)
		return NULL;

	ap=(void *)( (char *)0 + ((((char *)pv - (char *)0) + CACHE_LINE_BYTES - 1) & ~((unsigned long)CACHE_LINE_BYTES-1)) );

	clablocks[0][nblocks]=pv;
	clablocks[1][nblocks]=ap;
	nblocks++;

	return ap;
}


void clafree(void * addr)
{
	int i;

	nblocks--;
	for (i=nblocks; i>=0; i--)
		if (clablocks[1][i]==addr)
		{
			free(clablocks[0][i]);
			clablocks[0][i]=clablocks[0][nblocks];
			clablocks[1][i]=clablocks[1][nblocks];
			return;
		}

	free(NULL);
	return;
}


#endif
