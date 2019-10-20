#ifndef STATSTRUCT_H
#define STATSTRUCT_H

#include "cdfpred.h"

//extern
//int width,	/* img width */
//				/* in header stored as u16 */
//	height,	/* height */
//				/* in header stored as u16 */
//	bpp;	/* depth */
//				/* in header stored as u8 */


//int	echo=0,				/* commented in taskparams.h */
//	f_compress,
//	noheader=0,
//	decoronly=0,
//	generic8bpp=0;

//char * infilename,
//	 * outfilename;

//int width,
//	height,
//	bpp;


typedef unsigned COUNTER;	/* counter in the array of counters in bucket of the data model */

typedef struct s_bucket		/* bucket */
{
	COUNTER			* pcounters;	/* pointer to array of counters */
	unsigned int	bestcode;		/* best code so far */
} s_bucket;

typedef s_bucket * p_s_bucket;	/* pointer to bucket */

typedef struct SFALICData
{
	COUNTER * pc;	/* helper for (de)allocation of arrays of counters */

	s_bucket * pb;	/* array of counters */

	unsigned int		 wm_trigger;	/* threshold for halving counters, if trigger from taskparams <>0 */
	p_s_bucket			*b_ptr_lo;	/* array of pointers to buckets - low part */
	p_s_bucket			*b_ptr_hi;	/* array of pointers to buckets - high part */
	unsigned int		 b_lo_ptrs;		/* size of b_lo_ptrs */
	unsigned int		 b_hi_ptrs;
	unsigned int		 waitcnt;		/* global counter of skipped model updates */
	unsigned int		 _bpp;			/* local copy of bpp from taskparams */

	/* defaults for below externs are in cdfpred.h and cdfstat.h */
	/* definitions and initializations in taskparams.c */

	int pred,		/* predictor number */
	/* NOTE: in header stored as i8 (signed) */
	evol,		/* model evolution mode */
	/* in header stored as u8 */
	trigger,	/* bucket trigger */
	/* NOTE: in header trigger/10 is stored as u8 */
	maxclen,	/* codeword length limit */
	/* in header stored as u8 */
	wmistart,	/* starting wait mask index */
	/* in header stored as u8 */
	wmimax, 	/* target wait mask index */
	/* in header stored as u8 */
	wminext;	/* number of symbols to encode before increasing wait mask index */
	/* NOTE: in header log_2(wminext) is stored as u8 */

	/* set by findmodelparams(), used by statfillstructures() */
	unsigned int
	repfirst,		/* number of buckets of size equal to the 0th one */
	firstsize,		/* size of the 0th bucket */
	repnext,		/* how many consecutive buckets are of the same size */
	mulsize,		/* how many times to increase the size of the bucket */
	levels;			/* number of intensity levels */
	unsigned int
	nbuckets;		/* number of buckets */

	unsigned int
	ncounters;		/* number of counters allocated per bucket */

	unsigned int
	wmidx,		/* current wait mask index */
	wmileft;	/* how many symbols to encode using current wmidx */

	int	echo,		/* be verbose? */
		f_compress,	/* 1 compress, 0 decompress*/
		noheader,	/* skip writting output file header? */
		decoronly,	/* skip stsistical compression? - for testing only, obviously cannot be stored in the output file ;) */
		generic8bpp;/* use generic routines (16bpp) for up to 8bpp depths also? */
} SFALICData;

void initSFALICData( SFALICData *pEncDat );

#endif // STATSTRUCT_H
