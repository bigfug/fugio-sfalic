/* malloc/calloc/free equivalents */
/* clamalloc/clacalloc return addresses being integer multiplicities of CACHE_LINE_BYTES */
/* clafree is slow if executed not in lifo order in respect to cla?alloc's */
/* use clafree to free mem allocated by clamalloc/clacalloc*/
/* macro CLALLOC_OFF turns the module off */

#ifndef __CLALLOC_H
#define __CLALLOC_H

#include <stdlib.h>


/* #define CLALLOC_OFF */


#ifndef CLALLOC_OFF


/* length of the cache line */
#define CACHE_LINE_BYTES 128

/* number of active allocations allowed */
#define CLALLOC_TAB_SIZE 16

void *clacalloc(size_t num, size_t size);
void *clamalloc(size_t bytes);
void clafree(void * addr);


#else


#define clacalloc calloc
#define clamalloc malloc
#define clafree free


#endif	/* CLALLOC_OFF */


#endif

