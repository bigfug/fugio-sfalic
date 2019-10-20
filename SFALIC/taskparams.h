#ifndef __CDFTASKPARAMS_H
#define __CDFTASKPARAMS_H

#include "str.h"

/* parameters of the compression process, etc. */

/* version number of the compressor BYTE */
#define SFALICVERSION 04

/* version number of the compressed file format, BYTE */
#define FILEFORMATVERSION 03

/* name of the compression program plus version number, ASCIIZ */
#define PROGNAME "SFALIC research ver. " STR(SFALICVERSION) 


/* image width limit (and image height limit also) */
#define IMG_MAX_DIM 30000


/* parameters retrieved from commandline and from the input file header */

//extern
//int	echo,		/* be verbose? */
//	f_compress,	/* 1 compress, 0 decompress*/
//	noheader,	/* skip writting output file header? */
//	decoronly,	/* skip stsistical compression? - for testing only, obviously cannot be stored in the output file ;) */
//	generic8bpp;/* use generic routines (16bpp) for up to 8bpp depths also? */

//extern
//char * infilename,
//	 * outfilename;

//extern
//int width,	/* img width */
//				/* in header stored as u16 */
//	height,	/* height */
//				/* in header stored as u16 */
//	bpp;	/* depth */
//				/* in header stored as u8 */

/* defaults for below externs are in cdfpred.h and cdfstat.h */
/* definitions and initializations in taskparams.c */
//extern
//int pred,		/* predictor number */
//					/* NOTE: in header stored as i8 (signed) */
//	evol,		/* model evolution mode */
//					/* in header stored as u8 */
//	trigger,	/* bucket trigger */
//					/* NOTE: in header trigger/10 is stored as u8 */
//	maxclen,	/* codeword length limit */
//					/* in header stored as u8 */
//	wmistart,	/* starting wait mask index */
//					/* in header stored as u8 */
//	wmimax, 	/* target wait mask index */
//					/* in header stored as u8 */
//	wminext;	/* number of symbols to encode before increasing wait mask index */
//					/* NOTE: in header log_2(wminext) is stored as u8 */

/* output params to stdout */
void printparams();

/* check if params acceptable, otherwise exitit()*/
void testparams();

#endif
