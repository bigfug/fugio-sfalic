/* statistical compression */

/* time-critical part */

#include "tabrand.h"
#include "cdfstat.h"
#include "cfamily.h"
#include <assert.h>
#include <stdlib.h>
#include "memory.h"

#include "encodecodeword.h"
#include "decodecodeword.h"


/* used by findbucket() */
//static p_s_bucket * b_ptr_lo;	/* array of pointers to buckets - low part */
//static p_s_bucket * b_ptr_hi;	/* array of pointers to buckets - high part */
//static unsigned int
//b_lo_ptrs;		/* size of b_lo_ptrs */
//static unsigned int
//b_hi_ptrs;

///* used by updatemodel() */
//static unsigned int
//wm_trigger;	/* threshold for halving counters, if trigger from taskparams <>0 */
///* then wm_trigger=trigger, else wm_trigger is determined by set_wm_trigger()*/

///* used by w statdecompressrow() i statcompressrow() */
//static unsigned int
//waitcnt;		/* global counter of skipped model updates */

//static unsigned int
//_bpp;			/* local copy of bpp from taskparams */


/* return pointer to bucket containing context val */
s_bucket * findbucket( SFALICData *pEncDat, const unsigned int val )
{
	//assert(val<(0x1U<<_bpp));

	if( !(val<(0x1U<<pEncDat->_bpp)) )
	{
		return( 0 );
	}

	if( val<pEncDat->b_lo_ptrs )
		return pEncDat->b_ptr_lo[val];
	else
		return pEncDat->b_ptr_hi[(val-pEncDat->b_lo_ptrs)>>8];
}


/* update the bucket using just encoded curval */
static void updatemodel( SFALICData *pEncDat, s_bucket * const bucket, const unsigned int curval,
						const unsigned int bpp)
{ 
	COUNTER * const pcounters=bucket->pcounters;
	unsigned int i;
	unsigned int bestcode;
	unsigned int bestcodelen;

	/* update counters, find minimum */

	bestcode=bpp-1;
	bestcodelen=( pcounters[bestcode]+=GolombCodeLen(curval, bestcode) );

	for (i=bpp-2; i<bpp; i--) /* NOTE: expression i<bpp for signed int i would be: i>=0 !!! */
	{
		const unsigned int ithcodelen=( pcounters[i]+=GolombCodeLen(curval, i) );

		if (ithcodelen<bestcodelen)
		{
			bestcode=i;
			bestcodelen=ithcodelen;
		}
	}

	bucket->bestcode=bestcode; /* store the found minimum */

	if(bestcodelen>pEncDat->wm_trigger)	/* halving counters? */
		for (i=0; i<bpp; i++)
			pcounters[i]>>=1;
}


/* for const waitmask perform compression */
/* args: context of the first symbol, array of symbols, length of the array, waitmask, */
/* output buffer and status vars for output buffer */
#ifndef ALT_STATCOMPRESSROWWM
static void statcompressrowwm(SFALICData *pEncDat, const PIXEL firstcontext, const PIXEL uncompressedrow[],
							  const unsigned int width, const unsigned int waitmask,
							  BYTE compressedrow[],
							  unsigned int * const fullbytes, unsigned int * const bitsused)
{
	unsigned int i;

	unsigned int stopidx; /* inner loop limit */

	ENCODE_START(compressedrow, fullbytes, bitsused)


			assert(width);

	{	/* 0th */
		unsigned int codeword, codewordlen ;

		GolombCoding(uncompressedrow[0],
				findbucket( pEncDat, firstcontext)->bestcode,
				&codeword, &codewordlen);
		ENCODE(codeword, codewordlen)

				if (pEncDat->waitcnt)
				pEncDat->waitcnt--;
		else
		{
			pEncDat->waitcnt=(tabrand() & waitmask);
			updatemodel(pEncDat, findbucket( pEncDat, firstcontext),
						uncompressedrow[0] , pEncDat->_bpp);
		}
	}

	i=1;
	stopidx=i+pEncDat->waitcnt;

	while (stopidx<width) /* encode sequence of symbols, update model after last symbol */
	{
		for (; i<=stopidx; i++)
		{
			unsigned int codeword, codewordlen ;

			GolombCoding(uncompressedrow[i],
						 findbucket(pEncDat, uncompressedrow[i-1])->bestcode,
					&codeword, &codewordlen);
			ENCODE(codeword, codewordlen)
		}

		updatemodel(pEncDat, findbucket( pEncDat, uncompressedrow[stopidx-1]),
				uncompressedrow[stopidx] , pEncDat->_bpp);
		stopidx=i+(tabrand() & waitmask);
	}

	for (; i<width; i++)	/* last sequence, not followed by model update */
	{
		unsigned int codeword, codewordlen ;

		GolombCoding(uncompressedrow[i],
					 findbucket(pEncDat, uncompressedrow[i-1])->bestcode,
				&codeword, &codewordlen);
		ENCODE(codeword, codewordlen)
	}

	ENCODE_STOP(compressedrow, fullbytes, bitsused)

			pEncDat->waitcnt=stopidx-width;
}
#else
static void statcompressrowwm(PIXEL context, const PIXEL uncompressedrow[],
							  const unsigned int width, const unsigned int waitmask,
							  BYTE compressedrow[],
							  unsigned int * const fullbytes, unsigned int * const bitsused)
{
	unsigned int i;

	unsigned int stopidx; /* inner loop limit */

	ENCODE_START(compressedrow, fullbytes, bitsused)


			assert(width);

	{	/* 0th */
		unsigned int codeword, codewordlen ;
		struct s_bucket * pbucket=findbucket(context);

		GolombCoding(context=uncompressedrow[0],
					 pbucket->bestcode,
					 &codeword, &codewordlen);
		ENCODE(codeword, codewordlen)

				if (waitcnt)
				waitcnt--;
		else
		{
			waitcnt=(tabrand() & waitmask);
			updatemodel(pbucket, context, _bpp);
		}
	}

	i=1;
	stopidx=i+waitcnt;

	while (stopidx<width) /* encode sequence of symbols, update model after last symbol */
	{
		struct s_bucket * pbucket;

		for (; i<=stopidx; i++)
		{
			unsigned int codeword, codewordlen ;
			pbucket=findbucket(context);

			GolombCoding(context=uncompressedrow[i],
						 pbucket->bestcode,
						 &codeword, &codewordlen);
			ENCODE(codeword, codewordlen)
		}

		updatemodel(pbucket, context, _bpp);
		stopidx=i+(tabrand() & waitmask);
	}

	for (; i<width; i++)	/* last sequence, not followed by model update */
	{
		unsigned int codeword, codewordlen ;
		struct s_bucket * pbucket=findbucket(context);

		GolombCoding(context=uncompressedrow[i],
					 pbucket->bestcode,
					 &codeword, &codewordlen);
		ENCODE(codeword, codewordlen)
	}

	ENCODE_STOP(compressedrow, fullbytes, bitsused)

			waitcnt=stopidx-width;
}
#endif


s_bucket * findbucket8bpp( SFALICData *pEncDat, const unsigned int val)
{
	//assert(val<(0x1U<<_bpp));

	if( !(val<(0x1U<<pEncDat->_bpp)) )
	{
		return( 0 );
	}

	return pEncDat->b_ptr_lo[val];
}


#ifndef ALT_STATCOMPRESSROWWM
static void statcompressrowwm8bpp(SFALICData *pEncDat, const BYTE firstcontext, const BYTE uncompressedrow[],
								  const unsigned int width, const unsigned int waitmask,
								  BYTE compressedrow[],
								  unsigned int * const fullbytes, unsigned int * const bitsused)
{
	unsigned int i;
	unsigned int stopidx;

	ENCODE_START(compressedrow, fullbytes, bitsused)

			assert(width);

	{
		unsigned int codeword, codewordlen ;

		GolombCoding(uncompressedrow[0],
				findbucket8bpp(pEncDat, firstcontext)->bestcode,
				&codeword, &codewordlen);
		ENCODE(codeword, codewordlen)

				if (pEncDat->waitcnt)
				pEncDat->waitcnt--;
		else
		{
			pEncDat->waitcnt=(tabrand() & waitmask);
			updatemodel(pEncDat, findbucket8bpp(pEncDat, firstcontext),
						uncompressedrow[0] , pEncDat->_bpp);
		}
	}

	i=1;
	stopidx=i+pEncDat->waitcnt;

	while (stopidx<width)
	{
		for (; i<=stopidx; i++)
		{
			unsigned int codeword, codewordlen ;

			GolombCoding(uncompressedrow[i],
						 findbucket8bpp(pEncDat, uncompressedrow[i-1])->bestcode,
					&codeword, &codewordlen);
			ENCODE(codeword, codewordlen)
		}

		updatemodel(pEncDat, findbucket8bpp(pEncDat, uncompressedrow[stopidx-1]),
				uncompressedrow[stopidx] , pEncDat->_bpp);
		stopidx=i+(tabrand() & waitmask);
	}

	for (; i<width; i++)
	{
		unsigned int codeword, codewordlen ;

		GolombCoding(uncompressedrow[i],
					 findbucket8bpp(pEncDat, uncompressedrow[i-1])->bestcode,
				&codeword, &codewordlen);
		ENCODE(codeword, codewordlen)
	}

	ENCODE_STOP(compressedrow, fullbytes, bitsused)

			pEncDat->waitcnt=stopidx-width;
}
#else
static void statcompressrowwm8bpp(BYTE context, const BYTE uncompressedrow[],
								  const unsigned int width, const unsigned int waitmask,
								  BYTE compressedrow[],
								  unsigned int * const fullbytes, unsigned int * const bitsused)
{
	unsigned int i;
	unsigned int stopidx;

	ENCODE_START(compressedrow, fullbytes, bitsused)

			assert(width);

	{
		unsigned int codeword, codewordlen ;
		struct s_bucket * pbucket=findbucket8bpp(context);

		GolombCoding(context=uncompressedrow[0],
					 pbucket->bestcode,
					 &codeword, &codewordlen);
		ENCODE(codeword, codewordlen)

				if (waitcnt)
				waitcnt--;
		else
		{
			waitcnt=(tabrand() & waitmask);
			updatemodel(pbucket, context, _bpp);
		}
	}

	i=1;
	stopidx=i+waitcnt;

	while (stopidx<width)
	{
		struct s_bucket * pbucket;

		for (; i<=stopidx; i++)
		{
			unsigned int codeword, codewordlen ;
			pbucket=findbucket8bpp(context);

			GolombCoding(context=uncompressedrow[i],
						 pbucket->bestcode,
						 &codeword, &codewordlen);
			ENCODE(codeword, codewordlen)
		}

		updatemodel(pbucket, context, _bpp);
		stopidx=i+(tabrand() & waitmask);
	}

	for (; i<width; i++)
	{
		unsigned int codeword, codewordlen ;
		struct s_bucket * pbucket=findbucket8bpp(context);

		GolombCoding(context=uncompressedrow[i],
					 pbucket->bestcode,
					 &codeword, &codewordlen);
		ENCODE(codeword, codewordlen)
	}

	ENCODE_STOP(compressedrow, fullbytes, bitsused)

			waitcnt=stopidx-width;
}
#endif

/* ***************************************** decompress */

static int statdecompressrowwm(SFALICData *pDecDat, unsigned int context, PIXEL uncompressedrow[],
							   const unsigned int width, const unsigned int waitmask,
							   struct bitinstatus *bs)
{
	unsigned int i;
	unsigned int stopidx;
	DECODE_START(bs)

			i=0;
	stopidx=i+pDecDat->waitcnt;

	while (stopidx<width)
	{
		struct s_bucket * pbucket;

		for (; i<=stopidx; i++)
		{
			unsigned int codewordlen;

			pbucket=findbucket(pDecDat, context);
			context=GolombDecoding(pbucket->bestcode, bits, &codewordlen);
			uncompressedrow[i]=context;
			DECODE_EATBITS(codewordlen)
		}
		
		updatemodel(pDecDat, pbucket, context, pDecDat->_bpp);

		stopidx=i+(tabrand() & waitmask);
	}

	for (; i<width; i++)
	{
		unsigned int codewordlen;
		struct s_bucket * pbucket;

		pbucket=findbucket(pDecDat, context);
		context=GolombDecoding(pbucket->bestcode, bits, &codewordlen);
		uncompressedrow[i]=context;
		DECODE_EATBITS(codewordlen)
	}

	pDecDat->waitcnt=stopidx-width;

	DECODE_STOP(bs)

			return 0;
}


static int statdecompressrowwm8bpp( SFALICData *pDecDat, unsigned int context, BYTE uncompressedrow[],
								   const unsigned int width, const unsigned int waitmask,
								   struct bitinstatus *bs)
{
	unsigned int i;
	unsigned int stopidx;
	DECODE_START(bs)

			i=0;
	stopidx=i+pDecDat->waitcnt;

	while (stopidx<width)
	{
		struct s_bucket * pbucket;

		for (; i<=stopidx; i++)
		{
			unsigned int codewordlen;

			pbucket=findbucket8bpp(pDecDat, context);
			context=GolombDecoding(pbucket->bestcode, bits, &codewordlen);
			uncompressedrow[i]=context;
			DECODE_EATBITS(codewordlen)
		}
		
		updatemodel(pDecDat, pbucket, context, pDecDat->_bpp);

		stopidx=i+(tabrand() & waitmask);
	}

	for (; i<width; i++)
	{
		unsigned int codewordlen;
		struct s_bucket * pbucket;

		pbucket=findbucket8bpp(pDecDat, context);
		context=GolombDecoding(pbucket->bestcode, bits, &codewordlen);
		uncompressedrow[i]=context;
		DECODE_EATBITS(codewordlen)
	}

	pDecDat->waitcnt=stopidx-width;

	DECODE_STOP(bs)

			return 0;
}


/* not time-critical */


#include "cdftypes.h"
#include "exitit.h"
#include "bppmask.h"
#include "ceillog2.h"
#include "taskparams.h" 
#include "clalloc.h"


//static COUNTER * pc=NULL;	/* helper for (de)allocation of arrays of counters */

//static s_bucket * pb=NULL;	/* array of counters */


/* set by findmodelparams(), used by statfillstructures() */
//static unsigned int
//repfirst,		/* number of buckets of size equal to the 0th one */
//firstsize,		/* size of the 0th bucket */
//repnext,		/* how many consecutive buckets are of the same size */
//mulsize,		/* how many times to increase the size of the bucket */
//levels;			/* number of intensity levels */
//static unsigned int
//nbuckets;		/* number of buckets */

//static unsigned int
//ncounters;		/* number of counters allocated per bucket */

//static unsigned int
//wmidx,		/* current wait mask index */
//wmileft;	/* how many symbols to encode using current wmidx */

static const unsigned short besttrigtab[3][11]={ /* array of wm_trigger for waitmask and evol, used by set_wm_trigger() */
												 /* 1 */	{ 550, 900, 800, 700, 500, 350, 300, 200, 180, 180, 160 },
												 /* 3 */	{ 110, 550, 900, 800, 550, 400, 350, 250, 140, 160, 140 },
												 /* 5 */	{ 100, 120, 550, 900, 700, 500, 400, 300, 220, 250, 160 }
											   };



/* set wm_trigger knowing waitmask (param) and evol (glob)*/
void set_wm_trigger( SFALICData *pEncDat, unsigned int wm)
{
	if (pEncDat->trigger)
	{
		pEncDat->wm_trigger=pEncDat->trigger;
		return;
	}

	if (wm>10)
		wm=10;

#if defined( Q_DEBUG )
	assert(evol<6);
#endif

	pEncDat->wm_trigger=besttrigtab[pEncDat->evol/2][wm];

#if defined( Q_DEBUG )
	assert(wm_trigger<=2000);
	assert(wm_trigger>=1);
#endif
}


/* sets model params: ncodes, modelrows i finalbuckets*/
/* and model initialization params for statfillstructures(): repfirst/next, mulsize i evoluted */
void findmodelparams(SFALICData *pEncDat, const int bpp, const int evol)
{
	unsigned int
			bsize,			/* bucket size */
			bstart, bend,	/* bucket start and end, range : 0 to levels-1*/
			repcntr;		/* helper */

	//assert((bpp<=16) && (bpp>0));

	pEncDat->_bpp=bpp;

	if (bpp>8)
		pEncDat->ncounters=16;
	else
		pEncDat->ncounters=8;

	pEncDat->levels=0x1<<bpp;

	pEncDat->b_lo_ptrs=0;	/* ==0 means: not set yet */

	switch (evol)	/* set repfirst firstsize repnext mulsize */
	{
		case 1:						/* buckets contain following numbers of contexts:	1 1 1 2 2 4 4 8 8 ... */
			pEncDat->repfirst=3;	pEncDat->firstsize=1; pEncDat->repnext=2;	pEncDat->mulsize=2; break;
		case 3:						/* 	1 2 4 8 16 32 64 ... */
			pEncDat->repfirst=1;	pEncDat->firstsize=1; pEncDat->repnext=1;	pEncDat->mulsize=2; break;
		case 5:						/* 	1 4 16 64 256 1024 4096 16384 65536 */
			pEncDat->repfirst=1;	pEncDat->firstsize=1; pEncDat->repnext=1;	pEncDat->mulsize=4; break;
		case 0:						/* obsolete */
		case 2:						/* obsolete */
		case 4:						/* obsolete */
//			assert(0);
//			exitit("findmodelparams(): evol value obsolete!!!", 100);
		default:
//			assert(0);
//			exitit("findmodelparams(): evol out of range!!!", 100);
			return;
	}

	pEncDat->nbuckets=0;
	repcntr=pEncDat->repfirst+1;	/* first bucket */
	bsize=pEncDat->firstsize;

	do	/* other buckets */
	{
		if (pEncDat->nbuckets)			/* bucket start */
			bstart=bend+1;
		else
			bstart=0;

		if (!--repcntr)			/* bucket size */
		{
			repcntr=pEncDat->repnext;
			bsize*=pEncDat->mulsize;
		}

		bend=bstart+bsize-1;	/* bucket end */
		if (bend+bsize>=pEncDat->levels)	/* if following bucked was bigger than current one */
			bend=pEncDat->levels-1;		/* concatenate them */

		if (!pEncDat->b_lo_ptrs)			/* array size not set yet? */
		{
			if (bend==pEncDat->levels-1)		/* this bucket is last - all in the first array */
			{
				pEncDat->b_lo_ptrs=pEncDat->levels;
			}
			else if (bsize>=256)	/* this bucket is allowed to reside in the 2nd table */
			{
				pEncDat->b_lo_ptrs=bstart;
				//assert(bstart);		/* previous bucket exists */
			}
		}

		pEncDat->nbuckets++;
	}
	while (bend < pEncDat->levels-1);

	pEncDat->b_hi_ptrs=(255+pEncDat->levels-pEncDat->b_lo_ptrs)>>8; /* == ceil((levels-b_lo_ptrs)/256) */
}


/* fill the model */
/* counters are set to 0 by caloc()*/
int statfillstructures( SFALICData *pEncDat )
{
	unsigned int
			bsize,
			bstart,
			bend,
			repcntr,
			bnumber;

	COUNTER * freecounter=pEncDat->pc;	/* first free location in the array of counters */

	bnumber=0;

	repcntr=pEncDat->repfirst+1;	/* first bucket */
	bsize=pEncDat->firstsize;

	do	/* others */
	{
		if (bnumber)
			bstart=bend+1;
		else
			bstart=0;

		if (!--repcntr)
		{
			repcntr=pEncDat->repnext;
			bsize*=pEncDat->mulsize;
		}

		bend=bstart+bsize-1;
		if (bend+bsize>=pEncDat->levels)
			bend=pEncDat->levels-1;

		pEncDat->pb[bnumber].bestcode=pEncDat->_bpp-1;
		pEncDat->pb[bnumber].pcounters=freecounter;
		freecounter+=pEncDat->ncounters;

		if (bstart<pEncDat->b_lo_ptrs)
		{
			unsigned int i;
			//assert(bend<b_lo_ptrs);
			if( !(bend<pEncDat->b_lo_ptrs) ) return( 0 );

			for(i=bstart; i<=bend; i++)
				pEncDat->b_ptr_lo[i]=pEncDat->pb+bnumber;
		}
		else	/* high part */
		{
			unsigned int i,iend;
			//assert(bend>=b_lo_ptrs);

			if( !(bend>=pEncDat->b_lo_ptrs) ) return( 0 );

			i=(bstart-pEncDat->b_lo_ptrs)>>8;
			iend=(bend-pEncDat->b_lo_ptrs)>>8;
			for(;i<=iend; i++)
				pEncDat->b_ptr_hi[i]=pEncDat->pb+bnumber;
		}

		bnumber++;
	}
	while (bend < pEncDat->levels-1);

	//assert(freecounter-pc==nbuckets*ncounters);

	if( !(freecounter-pEncDat->pc==pEncDat->nbuckets*pEncDat->ncounters) ) return( 0 );

	return( 1 );
}


int statinitmodel( SFALICData *pEncDat, const int bpp, const int evol, const int init8bpp)
{
//	assert(!pb);
//	assert(!pc);
//	assert(bpp>=1);
//	assert(bpp<=16);

	findmodelparams(pEncDat, bpp, evol);

	if(init8bpp)
	{
//		assert(!b_hi_ptrs);
//		assert(b_lo_ptrs==(0x1U<<_bpp));
	}

//	assert(b_lo_ptrs);

	pEncDat->b_ptr_lo=(p_s_bucket *)clamalloc(pEncDat->b_lo_ptrs * sizeof(p_s_bucket));
	if (!pEncDat->b_ptr_lo)
	{
		//exitit("no memory for model allocation", 4);
		return( 0 );
	}

	if(pEncDat->b_hi_ptrs) /* it may be ==0 */
	{
		pEncDat->b_ptr_hi=(p_s_bucket *)clamalloc(pEncDat->b_hi_ptrs * sizeof(p_s_bucket));
		if (!pEncDat->b_ptr_hi)
		{
			//exitit("no memory for model allocation", 4);
			return( 0 );
		}
	}

	pEncDat->pb=(s_bucket *)clamalloc( (size_t)pEncDat->nbuckets * sizeof(s_bucket) );
	if (!pEncDat->pb)
	{
		//exitit("no memory for model allocation", 4);
		return( 0 );
	}

	/* memory for counters, not malloc, but calloc to set them to 0*/
	pEncDat->pc=(COUNTER *)clacalloc( (size_t)pEncDat->nbuckets, sizeof(COUNTER) * MAXNUMCODES );
	if (!pEncDat->pc)
	{
//		exitit("no memory for model allocation", 4);
		return( 0 );
	}

	return( statfillstructures( pEncDat ) );
}


void statfreemodel( SFALICData *pEncDat )
{
	if( pEncDat->pc )
	{
		clafree(pEncDat->pc);
		pEncDat->pc=NULL;
	}

	if( pEncDat->pb )
	{
		clafree(pEncDat->pb);
		pEncDat->pb=NULL;
	}

	if(pEncDat->b_hi_ptrs)
	{
		clafree(pEncDat->b_ptr_hi);
	}

	clafree(pEncDat->b_ptr_lo);
}

/* exported functions */

SFALICData *statinitcoder(const int bpp, const int maxclen,
								 const int evol, const int init8bpp)
{
	SFALICData		*EncDat = malloc( sizeof( SFALICData ) );

	if( !EncDat )
	{
		return( 0 );
	}

	memset( EncDat, 0, sizeof( SFALICData ) );

	initSFALICData( EncDat );

	//assert(bpp<=MAXNUMCODES);

	if( bpp > MAXNUMCODES )
	{
		return( 0 );
	}

	statinitmodel( EncDat, bpp, evol, init8bpp );

	familyinit(bpp, maxclen);

	stabrand();

	EncDat->wmidx=EncDat->wmistart;
	set_wm_trigger(EncDat, EncDat->wmidx);
	EncDat->wmileft=EncDat->wminext;
	EncDat->waitcnt=0;

	return( EncDat );
}


void statfreecoder( SFALICData *pEncDat )
{
	statfreemodel( pEncDat );

	free( pEncDat );
}

SFALICData *statinitdecoder(const int bpp, const int maxclen, const int evol, const int width, struct bitinstatus *bs)
{
	SFALICData		*EncDat = malloc( sizeof( SFALICData ) );

	if( !EncDat )
	{
		return( 0 );
	}

	memset( EncDat, 0, sizeof( SFALICData ) );

	initSFALICData( EncDat );

	//assert(bpp<=MAXNUMCODES);

	if( bpp > MAXNUMCODES )
	{
		return( 0 );
	}

	statinitmodel(EncDat, bpp, evol, 0);
	familyinit(bpp, maxclen);
	stabrand();
	EncDat->wmidx=EncDat->wmistart;
	set_wm_trigger(EncDat, EncDat->wmidx );
	EncDat->wmileft=EncDat->wminext;
	EncDat->waitcnt=0;

	bitinstatusinit(bs);

	return( EncDat );
}


void statfreedecoder( SFALICData *pDecDat )
{
	statfreemodel( pDecDat );

	free( pDecDat );
}


void statcompressrow( SFALICData *pEncDat, PIXEL context, const PIXEL * uncompressedrow, unsigned int width,
					 BYTE * compressedrow, unsigned int * fullbytes, unsigned int * bitsused)

{
	while ((pEncDat->wmimax > (int)pEncDat->wmidx) && (pEncDat->wmileft<=width)) /* wait mask index change inside the buffer */
	{
		if (pEncDat->wmileft)
		{
			statcompressrowwm(pEncDat, context, uncompressedrow, pEncDat->wmileft, bppmask[pEncDat->wmidx],
							  compressedrow, fullbytes, bitsused);
			context=*(uncompressedrow+pEncDat->wmileft-1);
			uncompressedrow+=pEncDat->wmileft;
			width-=pEncDat->wmileft;
		}

		pEncDat->wmidx++;
		set_wm_trigger(pEncDat, pEncDat->wmidx);
		pEncDat->wmileft=pEncDat->wminext;
	}

	if (width)
	{
		statcompressrowwm(pEncDat, context, uncompressedrow, width, bppmask[pEncDat->wmidx],
						  compressedrow, fullbytes, bitsused);
		if (pEncDat->wmimax > (int)pEncDat->wmidx)
			pEncDat->wmileft-=width;
	}

//	assert((int)wmidx<=wmimax);
//	assert(wmidx<=32);
//	assert(wminext>0);
}

void statcompressrow8bpp(SFALICData *pEncDat, BYTE context, const BYTE * uncompressedrow, unsigned int width,
						 BYTE * compressedrow, unsigned int * fullbytes, unsigned int * bitsused)

{
	while ((pEncDat->wmimax > (int)pEncDat->wmidx) && (pEncDat->wmileft<=width))
	{
		if (pEncDat->wmileft)
		{
			statcompressrowwm8bpp(pEncDat, context, uncompressedrow, pEncDat->wmileft, bppmask[pEncDat->wmidx],
								  compressedrow, fullbytes, bitsused);
			context=*(uncompressedrow+pEncDat->wmileft-1);
			uncompressedrow+=pEncDat->wmileft;
			width-=pEncDat->wmileft;
		}

		pEncDat->wmidx++;
		set_wm_trigger(pEncDat, pEncDat->wmidx);
		pEncDat->wmileft=pEncDat->wminext;
	}

	if (width)
	{
		statcompressrowwm8bpp(pEncDat, context, uncompressedrow, width, bppmask[pEncDat->wmidx],
							  compressedrow, fullbytes, bitsused);
		if (pEncDat->wmimax > (int)pEncDat->wmidx)
			pEncDat->wmileft-=width;
	}

//	assert((int)wmidx<=wmimax);
//	assert(wmidx<=32);
//	assert(wminext>0);
}

int statdecompressrow(SFALICData *pEncDat, PIXEL context, PIXEL * uncompressedrow, unsigned int width, struct bitinstatus *bs)
{
	int result;

	while ((pEncDat->wmimax > (int)pEncDat->wmidx) && (pEncDat->wmileft<=width))
	{
		if (pEncDat->wmileft)
		{
			result=statdecompressrowwm(pEncDat, context, uncompressedrow, pEncDat->wmileft, bppmask[pEncDat->wmidx], bs);
			context=*(uncompressedrow+pEncDat->wmileft-1);
			uncompressedrow+=pEncDat->wmileft;
			width-=pEncDat->wmileft;
		}

		pEncDat->wmidx++;
		set_wm_trigger(pEncDat, pEncDat->wmidx);
		pEncDat->wmileft=pEncDat->wminext;
	}

	if (width)
	{
		result=statdecompressrowwm(pEncDat, context, uncompressedrow, width, bppmask[pEncDat->wmidx], bs);
		if (pEncDat->wmimax > (int)pEncDat->wmidx)
			pEncDat->wmileft-=width;
	}

//	assert((int)wmidx<=wmimax);
//	assert(wmidx<=32);
//	assert(wminext>0);

	return result;
}


int statdecompressrow8bpp(SFALICData *pEncDat, BYTE context, BYTE * uncompressedrow, unsigned int width, struct bitinstatus *bs)
{
	int result;

	while ((pEncDat->wmimax > (int)pEncDat->wmidx) && (pEncDat->wmileft<=width))
	{
		if (pEncDat->wmileft)
		{
			result=statdecompressrowwm8bpp(pEncDat, context, uncompressedrow, pEncDat->wmileft, bppmask[pEncDat->wmidx], bs);
			context=*(uncompressedrow+pEncDat->wmileft-1);
			uncompressedrow+=pEncDat->wmileft;
			width-=pEncDat->wmileft;
		}

		pEncDat->wmidx++;
		set_wm_trigger(pEncDat, pEncDat->wmidx);
		pEncDat->wmileft=pEncDat->wminext;
	}

	if (width)
	{
		result=statdecompressrowwm8bpp(pEncDat, context, uncompressedrow, width, bppmask[pEncDat->wmidx], bs);
		if (pEncDat->wmimax > (int)pEncDat->wmidx)
			pEncDat->wmileft-=width;
	}

//	assert((int)wmidx<=wmimax);
//	assert(wmidx<=32);
//	assert(wminext>0);

	return result;
}

void initSFALICData( SFALICData *pEncDat )
{
	pEncDat->pred=DEFpred;
	pEncDat->evol=DEFevol;
	pEncDat->trigger=DEFtrigger;
	pEncDat->maxclen=DEFmaxclen;
	pEncDat->wmistart=DEFwmistart;
	pEncDat->wmimax=DEFwmimax;
	pEncDat->wminext=DEFwminext;
}
