#ifndef __CDFSTAT_H
#define __CDFSTAT_H

/* perform statistical compression and decompression */

#include "cdftypes.h"
#include "decodecodeword.h"

/* defining macro ALT_STATCOMPRESSROWWM may improve compression speed on some sytems (sun studio 10 compiler/UltraSPARC IIIi) */
/* #define ALT_STATCOMPRESSROWWM	*/



/* model evolution, warning: only 1,3 and 5 allowed */
#define DEFevol 3
#define MINevol 0
#define MAXevol 5

/* bucket trigger */
#define DEFtrigger 0
#define MINtrigger 0
#define MAXtrigger 2000 /* 2000 for u16b COUNTER and MAXmaxclen==32  */
/* over 2^26 for u32b COUNTER */
/* 0 - auto-select */

/* codeword length limit */
#define DEFmaxclen 26
/* MINmaxclen is image depth+1 */
#define MAXmaxclen 31

/* starting wait mask index */
#define DEFwmistart 0
#define MINwmistart 0
/* MAXwmistart is wmimax */

/* target wait mask index */
#define DEFwmimax 6
/* MINwmimax is wmistart */
#define MAXwmimax 15

/* number of symbols to encode before increasing wait mask index */
#define DEFwminext 2048
#define MINwminext 1
#define MAXwminext 100000000

#include "statstruct.h"

/* init. statistical coder */
/* uses params and data from taskparams.h */
SFALICData *statinitcoder(const int bpp, const int maxclen,
				   const int evol, const int init8bpp);

/* free statcoder structures */
void statfreecoder( SFALICData *pEncDat );

/* init decoder */
/* uses params and data from taskparams.h */
/* uses bs to init bit input */
SFALICData *statinitdecoder(const int bpp, const int maxclen,
					 const int evol, const int width,
					 struct bitinstatus *bs);

/* free statdecoder structures */
void statfreedecoder( SFALICData *pDecDat );

/* compress */
/* context - context of 0th prediction error, uncompressedrow - buffer containing pred. errors (0 to width-1) */
/* output compressed errors to compressedrow, fullbytes and bitsused store bit-output status */
void statcompressrow(SFALICData *pEncDat, PIXEL context, const PIXEL * uncompressedrow, unsigned int width,
					 BYTE * compressedrow, unsigned int * fullbytes, unsigned int * bitsused);
void statcompressrow8bpp(SFALICData *pEncDat, BYTE context, const BYTE * uncompressedrow, unsigned int width,
						 BYTE * compressedrow, unsigned int * fullbytes, unsigned int * bitsused);

/* decompress */
/* context - context of 0th prediction error, uncompressedrow - buffer for decompressed pred. errors (0 to width-1) */
/* bs stores the bit-input status */
/* return 0-ok, 1-error */
int statdecompressrow(SFALICData *pEncDat, PIXEL context, PIXEL * uncompressedrow, unsigned int width, struct bitinstatus *bs);
int statdecompressrow8bpp(SFALICData *pEncDat, BYTE context, BYTE * uncompressedrow, unsigned int width, struct bitinstatus *bs);

#endif
