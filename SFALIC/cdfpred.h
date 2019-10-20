#ifndef __CDFPRED_H
#define __CDFPRED_H

/* decorrelation and correlation */

#include "cdftypes.h"

/* predictor's range and the default predictor */
#define MINpred -1
#define MAXpred 8
#define DEFpred 8

/*	available predictors:

-1: raw binary copy

0: p=0; break;
1: p=a; break;
2: p=b; break;
3: p=c; break;
4: p=a+b-c; break;
5: p=(2*a+b-c)/2; break;  
6: p=(2*b+a-c)/2; break;  
7: p=(a+b)/2; break;
8: p=(3*a+3*b-2*c)/4; break;

*/

/* initialize decorrelation translation static array - for 8bpp routines */
void decorelateinit8bpp(int bpp);

/* decorrelation, row 0 - aware */
/* params: pevious and current row, current row number, row length, img depth, predictor number, buffer for decorrelated row */
void decorelaterow(const PIXEL *prevrow, const PIXEL *currow, int row, int rowlen, int bpp,
				   int pred, PIXEL *decorelatedrow);
/* BYTE-wise variant of decorelaterow, for depths up to 8bpp */
void decorelaterow8bpp(const BYTE *prevrow, const BYTE *currow, int row, int rowlen, int bpp,
				       int pred, BYTE *decorelatedrow);

/* initialize correlation translation static array - for 8bpp routines */
void corelateinit8bpp(int bpp);

/* reverse decorelaterow() */
void corelaterow(const PIXEL *prevrow, PIXEL *currow, int row, int rowlen, int bpp,
				   int pred, const PIXEL *decorelatedrow);
void corelaterow8bpp(const BYTE *prevrow, BYTE *currow, int row, int rowlen, int bpp,
				   int pred, const BYTE *decorelatedrow);



#endif
