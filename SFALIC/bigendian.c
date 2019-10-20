#include "bigendian.h"
#include "cdftypes.h"
#include "assert.h"
#include <memory.h>

int CheckAssumptions()
{
	volatile unsigned int ui=1;
#ifdef BE_MACHINE
	if (!BEmachine())
		return 1;
#endif

	ui++;
	if (ui!=(ui>>(ui-2))) /* is x==(x>>0), should be */
		return ui;

	return 0;
}


int BEmachine()
{
	static int one=1;

#ifdef BE_MACHINE
	assert(!*((BYTE *)&one));
#endif

	return !*((BYTE *)&one);
}

/* copy width pixels from src to dst inverting the endianness */
/* for little endian CPU only */
static void BEinvPIXELendianness(PIXEL * const dst, const PIXEL * const src, const int width)
{
	unsigned int i;
	const unsigned int wstop=(unsigned)width&~0x07;	/* stop for unrolled loop */

	assert(!BEmachine());

	for(i=0; i<wstop; i+=8)
	{
		const BYTE * const pb=(BYTE *)(src+i);
		PIXEL * const pdest=dst+i;
		pdest[0]=  (((PIXEL)pb[0] )<<8) | (pb[1] );
		pdest[1]=  (((PIXEL)pb[2] )<<8) | (pb[3] );
		pdest[2]=  (((PIXEL)pb[4] )<<8) | (pb[5] );
		pdest[3]=  (((PIXEL)pb[6] )<<8) | (pb[7] );
		pdest[4]=  (((PIXEL)pb[8] )<<8) | (pb[9] );
		pdest[5]=  (((PIXEL)pb[10])<<8) | (pb[11]);
		pdest[6]=  (((PIXEL)pb[12])<<8) | (pb[13]);
		pdest[7]=  (((PIXEL)pb[14])<<8) | (pb[15]);	
	}
	for(; i<(unsigned)width; i++)
		dst[i]=(src[i]>>8) | (src[i]<<8);

	return;
}

void BErawrowtopixelrow(const BYTE * const filerow, PIXEL * const currow, const int width, const int rawBYTESpp)
{
//	assert(width>0);
//	assert( ((PIXEL)-1) > 0 );
//	assert( sizeof(PIXEL) >= rawBYTESpp );
//	assert(sizeof(PIXEL)==2);

	if( width <= 0 ) return;
	if( ((PIXEL)-1) <= 0 ) return;
	if( sizeof(PIXEL) < (unsigned int)rawBYTESpp ) return;
	if( sizeof(PIXEL) != 2 ) return;

	if (rawBYTESpp==1) /* convert Bytes to Pixels */
	{
		unsigned int i;

		for(i=0; i<(unsigned)width; i++)
			currow[i]=(PIXEL)filerow[i];

		return;
	}
	
	if (!BEmachine())	/* Little Endian convert BE pixels to LE */
		BEinvPIXELendianness(currow, (PIXEL*)filerow, width);
	else				/* Big Endian CPU */
		memcpy(currow, filerow, sizeof(PIXEL)*width);
}


void BEpixelrowtorawrow(BYTE * const filerow, const PIXEL * const currow, const int width, const int rawBYTESpp)
{
//	assert(width>0);
//	assert( ((PIXEL)-1) > 0 );
//	assert( int( sizeof(PIXEL) ) >= rawBYTESpp );

//	assert(sizeof(PIXEL)==2);

	if( !(width>0) ) return;
	if( !(((PIXEL)-1) > 0) ) return;
	if( !(sizeof(PIXEL) >= (unsigned int)rawBYTESpp) ) return;
	if( !(sizeof(PIXEL)==2) ) return;

	if (rawBYTESpp==1)
	{
		unsigned int i;

		for(i=0; i<(unsigned)width; i++)
			filerow[i]=(BYTE)currow[i];

		return;
	}
	
	if (!BEmachine())
		BEinvPIXELendianness((PIXEL*)filerow, currow, width);
	else
		memcpy(filerow, currow, sizeof(PIXEL)*width);
}


int BEwrite2Bytes(int val, FILE *f)
{
	putc((BYTE)(val>>8), f);
	return putc((BYTE)val, f);
}


int BEread2Bytes(FILE *f)
{
	int result;
	result=(BYTE)getc(f);
	result<<=8;
	result|=(BYTE)getc(f);

	return result;
}


int BEwrite4Bytes(int val, FILE *f)
{
	putc((BYTE)(val>>24), f);
	putc((BYTE)(val>>16), f);
	putc((BYTE)(val>>8), f);

	return putc((BYTE)val, f);
}


int BEread4Bytes(FILE *f)
{
	int result;
	result=(BYTE)getc(f);
	result<<=8;
	result|=(BYTE)getc(f);
	result<<=8;
	result|=(BYTE)getc(f);
	result<<=8;
	result|=(BYTE)getc(f);

	return result;
}
