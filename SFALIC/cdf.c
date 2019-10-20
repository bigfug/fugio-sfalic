/* SFALIC main */

#include "cdftypes.h"
#include "taskparams.h"
#include "cdfcmdline.h"
#include "exitit.h"
#include "cdfpred.h"
#include "cdfstat.h"
#include "headers.h"
#include "bigendian.h"
#include "clalloc.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* minimum size of bit-input buffer for decompression() */
#define INBUFMIN 16384


void compression(FILE *infile, FILE *outfile)
{
	int fileBYTESpp;
	PIXEL context;  /* context of the 1st pixel in the row */
	int row;		/* current row nr */
	unsigned int bitsused;	/* for bit-io number of bits output and not written yet to the buffer */
	int needconvert;		/* is conversion from raw image data to pixel type required? */

	BYTE *filerow;	/* if conversion required - image row read from input */
	PIXEL *prevrow, /* previous row */
		  *currow,  /* current row */
		  *decorelatedrow;	/* decorrelated row */
	BYTE  *compressedrow;	/* compressed row - max len=4B/pixel+4B */

	if(!decoronly)
		statinitcoder(bpp, maxclen, evol, 0);

	if (bpp<9)
		fileBYTESpp=1;
	else 
		fileBYTESpp=2;

	needconvert=(fileBYTESpp!=sizeof(PIXEL)) || (fileBYTESpp>1 && !BEmachine());

	if (!(compressedrow=(BYTE *)clamalloc(4*(width+1))))
		exitit("error compressedrow buffer allocation", 4);
	if (!(prevrow=(PIXEL *)clamalloc(width*sizeof(PIXEL))))
		exitit("error pervrow buffer allocation", 4);
	if (!(currow=(PIXEL *)clamalloc(width*sizeof(PIXEL))))
		exitit("error currow buffer allocation", 4);
	if (!(decorelatedrow=(PIXEL *)clamalloc(width*sizeof(PIXEL))))
		exitit("error decorelatedrow buffer allocation", 4);
	if(needconvert)
		if (!(filerow=(BYTE *)clamalloc(width*fileBYTESpp)))
			exitit("error filerow buffer allocation", 4);

	context=0;
	bitsused=0;

	for (row=0; row<height; row++)
	{	
		PIXEL *helprowptr;

		helprowptr=prevrow;
		prevrow=currow;
		currow=helprowptr;

		if(needconvert)
		{
			if (width!=fread((void *)filerow, fileBYTESpp, width, infile))
				exitit("error reading pixels from infile", 4);
			BErawrowtopixelrow(filerow, currow, width, fileBYTESpp);
		}
		else
			if (width!=fread((void *)currow, fileBYTESpp, width, infile))
				exitit("error reading pixels from infile", 4);

		decorelaterow(prevrow, currow, row, width, bpp, pred, decorelatedrow);

		if (decoronly)
		{
			if (width!=fwrite((void *)decorelatedrow, sizeof(PIXEL), width, outfile))
				exitit("error writting decorelated pixels to outfile", 4);
		}
		else
		{
			unsigned int fullbytes=0;
			statcompressrow(context, decorelatedrow, width, 
							compressedrow, &fullbytes, &bitsused);
			if (fullbytes!=fwrite((void *)compressedrow, 1, fullbytes, outfile))
				exitit("error writting compressed pixels to outfile", 4);
			if (bitsused) /* bitsused is in range 0-31 */
				*(unsigned int *)compressedrow=*(unsigned int *)(compressedrow+fullbytes);
		}

		context=*decorelatedrow;
	}

	if(!decoronly)
	{
		if (bitsused)
			if (1!=fwrite((void *)compressedrow, (bitsused+7)/8, 1, outfile))
				exitit("error writting compressed pixels to outfile", 4);
		statfreecoder();
	}

	if(needconvert)
		clafree(filerow);
	clafree(decorelatedrow);
	clafree(currow);
	clafree(prevrow);
	clafree(compressedrow);
}


void compression8bpp(FILE *infile, FILE *outfile)
{
	BYTE context;
	int row;
	unsigned int bitsused;

	BYTE *prevrow,
		  *currow,
		  *decorelatedrow;
	BYTE  *compressedrow;

	assert(bpp<=8);

	decorelateinit8bpp(bpp);
	if(!decoronly)
		statinitcoder(bpp, maxclen, evol, 1);

	if (!(compressedrow=(BYTE *)clamalloc(4*(width+1))))
		exitit("error compressedrow buffer allocation", 4);
	if (!(prevrow=(BYTE *)clamalloc(width)))
		exitit("error pervrow buffer allocation", 4);
	if (!(currow=(BYTE *)clamalloc(width)))
		exitit("error currow buffer allocation", 4);
	if (!(decorelatedrow=(BYTE *)clamalloc(width)))
		exitit("error decorelatedrow buffer allocation", 4);

	context=0;
	bitsused=0;

	for (row=0; row<height; row++)
	{	
		BYTE *helprowptr;

		helprowptr=prevrow;
		prevrow=currow;
		currow=helprowptr;

		if (width!=fread((void *)currow, 1, width, infile))
			exitit("error reading pixels from infile", 4);

		decorelaterow8bpp(prevrow, currow, row, width, bpp, pred, decorelatedrow);

		if (decoronly)
		{
			if (width!=fwrite((void *)decorelatedrow, 1, width, outfile))
				exitit("error writting decorelated pixels to outfile", 4);
		}
		else
		{
			unsigned int fullbytes=0;
			statcompressrow8bpp(context, decorelatedrow, width, 
							compressedrow, &fullbytes, &bitsused);
			if (fullbytes!=fwrite((void *)compressedrow, 1, fullbytes, outfile))
				exitit("error writting compressed pixels to outfile", 4);
			if (bitsused)
				*(unsigned int *)compressedrow=*(unsigned int *)(compressedrow+fullbytes);
		}

		context=*decorelatedrow;
	}

	if(!decoronly)
	{
		if (bitsused)
			if (1!=fwrite((void *)compressedrow, (bitsused+7)/8, 1, outfile))
				exitit("error writting compressed pixels to outfile", 4);
		statfreecoder();
	}

	clafree(decorelatedrow);
	clafree(currow);
	clafree(prevrow);
	clafree(compressedrow);
}


void decompression(FILE *infile, FILE *outfile)
{
	int fileBYTESpp;
	int compressedrowlen=4*(width+2);
	PIXEL context;
	int row;
	struct bitinstatus bs;
	int needconvert;
	int eof=0;

	BYTE *filerow, 
		 *compressedrow;
	PIXEL *prevrow, 
		  *currow, 
		  *decorelatedrow;

	if (compressedrowlen<INBUFMIN)
		compressedrowlen=INBUFMIN;

	assert(!decoronly);

	if (bpp<9)
		fileBYTESpp=1;
	else 
		fileBYTESpp=2;

	needconvert=(fileBYTESpp!=sizeof(PIXEL)) || (fileBYTESpp>1 && !BEmachine());

	context=0;

	if (!(compressedrow=(BYTE *)clamalloc(compressedrowlen)))
		exitit("error compressedrow buffer allocation", 4);
	if (!(prevrow=(PIXEL *)clamalloc(width*sizeof(PIXEL))))
		exitit("error pervrow buffer allocation", 4);
	if (!(currow=(PIXEL *)clamalloc(width*sizeof(PIXEL))))
		exitit("error currow buffer allocation", 4);
	if (!(decorelatedrow=(PIXEL *)clamalloc(width*sizeof(PIXEL))))
		exitit("error decorelatedrow buffer allocation", 4);
	if (needconvert)
		if (!(filerow=(BYTE *)clamalloc(width*fileBYTESpp)))
			exitit("error filerow buffer allocation", 4);

	eof=(compressedrowlen!=fread((void *)compressedrow, 1, compressedrowlen, infile));
	bs.rdptr=compressedrow;
	statinitdecoder(bpp, maxclen, evol, width, &bs);

	for (row=0; row<height; row++)
	{	
		PIXEL *helprowptr;

		int usedbytes;
		if (statdecompressrow(context, decorelatedrow, width, &bs))
			exitit("error decompressing (statdecompressrow())", 4);
		usedbytes=(int)(bs.rdptr-compressedrow);
		if(usedbytes>compressedrowlen)
			exitit("error decompressing image", 4);
		if ((compressedrowlen-usedbytes<4*width) && (!eof)) /* input buffer may contain less bits than compressed image row - fill needed */
		{
			memmove(compressedrow, compressedrow+usedbytes, compressedrowlen-usedbytes);
			bs.rdptr=compressedrow;
			eof=(usedbytes!=fread((void *)(compressedrow+(compressedrowlen-usedbytes))
									, 1, usedbytes, infile));
		}

		context=*decorelatedrow;
		corelaterow(prevrow, currow, row, width, bpp, pred, decorelatedrow);

		if (needconvert)
		{
			BEpixelrowtorawrow(filerow, currow, width, fileBYTESpp);	
			if (width!=fwrite((void *)filerow, fileBYTESpp, width, outfile))	
				exitit("error writting pixels to outfile", 4);	
		}
		else
			if (width!=fwrite((void *)currow, fileBYTESpp, width, outfile))	
				exitit("error writting pixels to outfile", 4);	

		helprowptr=prevrow;
		prevrow=currow;
		currow=helprowptr;
	}

	statfreedecoder();

	if (needconvert)
		clafree(filerow);
	clafree(decorelatedrow);
	clafree(currow);
	clafree(prevrow);
	clafree(compressedrow);
}


void decompression8bpp(FILE *infile, FILE *outfile)
{
	int compressedrowlen=4*(width+2);
	BYTE context;
	int row;
	struct bitinstatus bs;		
	int eof=0;

	BYTE *compressedrow,
		 *prevrow, 
		 *currow, 
		 *decorelatedrow;

	if (compressedrowlen<INBUFMIN)
		compressedrowlen=INBUFMIN;

	assert(!decoronly);
	assert(bpp<=8);

	corelateinit8bpp(bpp); /* init. for 8bpp only */

	context=0;

	if (!(compressedrow=(BYTE *)clamalloc(compressedrowlen)))
		exitit("error compressedrow buffer allocation", 4);
	if (!(prevrow=(BYTE *)clamalloc(width)))
		exitit("error pervrow buffer allocation", 4);
	if (!(currow=(BYTE *)clamalloc(width)))
		exitit("error currow buffer allocation", 4);
	if (!(decorelatedrow=(BYTE *)clamalloc(width)))
		exitit("error decorelatedrow buffer allocation", 4);

	eof=(compressedrowlen!=fread((void *)compressedrow, 1, compressedrowlen, infile));
	bs.rdptr=compressedrow;
	statinitdecoder(bpp, maxclen, evol, width, &bs);

	for (row=0; row<height; row++)
	{	
		BYTE *helprowptr;

		int usedbytes;
		if (statdecompressrow8bpp(context, decorelatedrow, width, &bs))
			exitit("error decompressing (statdecompressrow())", 4);
		usedbytes=(int)(bs.rdptr-compressedrow);
		if(usedbytes>compressedrowlen)
			exitit("error decompressing image", 4);
		if ((compressedrowlen-usedbytes<4*width) && (!eof)) /* input buffer may contain less bits than compressed image row - fill needed */
		{
			memmove(compressedrow, compressedrow+usedbytes, compressedrowlen-usedbytes);
			bs.rdptr=compressedrow;
			eof=(usedbytes!=fread((void *)(compressedrow+(compressedrowlen-usedbytes))
									, 1, usedbytes, infile));
		}

		context=*decorelatedrow;

		corelaterow8bpp(prevrow, currow, row, width, bpp, pred, decorelatedrow);

		if (width!=fwrite((void *)currow, 1, width, outfile))	
			exitit("error writting pixels to outfile", 4);	


		helprowptr=prevrow;
		prevrow=currow;
		currow=helprowptr;
	}

	statfreedecoder();

	clafree(decorelatedrow);
	clafree(currow);
	clafree(prevrow);
	clafree(compressedrow);
}


int main(const int argc, char **argv)
{
	FILE *infile, *outfile;

	if (CheckAssumptions())
		exitit("internal error: CheckAssumptions() fail. Chceck compiler options and macros.", 100);

	processcmdline(argc, argv);

	if (echo) 
		printf("\n" PROGNAME);
	if (echo) 
		printf("\nBEmachine() = %d", BEmachine());

#ifndef BE_MACHINE
	if(BEmachine()) 
		printf("\nSFALIC Warning: for better speed recompile with macro BE_MACHINE defined.");
#endif
#ifndef NDEBUG
	printf("\nSFALIC Warning: for better speed recompile with macro NDEBUG defined.");
#endif

	infile=fopen(infilename, "rb");
	if (!infile)
		exitit("error openning infile", 2);
	if (f_compress)
		readPGMP5header(infile);
	else
		readCDFheader(infile);

	if (echo) 
		printparams();

	testparams();

	outfile=fopen(outfilename, "wb");
	if (!outfile)
		exitit("error openning outfile", 2);
	if (!noheader)
	{
		if (f_compress)
			writeCDFheader(outfile);
		else
			writePGMP5header(outfile);
	}

	if (f_compress)
	{
		if (bpp<=8 && !generic8bpp)
			compression8bpp(infile, outfile);
		else
			compression(infile, outfile);
	}
	else
	{
		if (bpp<=8 && !generic8bpp)
			decompression8bpp(infile, outfile);
		else
			decompression(infile, outfile);
	}

	fclose(infile);
	fclose(outfile);

	if (echo) 
		printf("\n\nFinished!\n");

	return 0;
}
