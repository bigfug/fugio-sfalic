#include "taskparams.h"
#include "cdfpred.h"
#include "cdfstat.h"
#include "cfamily.h"
#include "exitit.h"
#include "str.h"
#include "ceillog2.h"

#include <stdio.h>

int	echo=0,				/* commented in taskparams.h */
	f_compress,
	noheader=0,
	decoronly=0,
	generic8bpp=0;	

char * infilename,
	 * outfilename;

int width,	
	height,	
	bpp;	

int pred=DEFpred,	
	evol=DEFevol,
	trigger=DEFtrigger,
	maxclen=DEFmaxclen,
	wmistart=DEFwmistart,
	wmimax=DEFwmimax,
	wminext=DEFwminext;


void printparams()
{
	printf("\necho=%d", echo);
	printf("\nf_compress=%d", f_compress);
	printf("\nnoheader=%d", noheader);
	printf("\ndecoronly=%d", decoronly);
	printf("\ngeneric8bpp=%d", generic8bpp);
	printf("\ninfilename=%s", infilename);
	printf("\noutfilename=%s", outfilename);
	printf("\nFILEFORMATVERSION=%d", FILEFORMATVERSION);
	printf("\nSFALICVERSION=%d", SFALICVERSION);
	printf("\nwidth=%d", width);
	printf("\nheight=%d", height);
	printf("\nbpp=%d", bpp);
	printf("\npred=%d", pred);
	printf("\nevol=%d", evol);
	printf("\ntrigger=%d", trigger);
	printf("\nmaxclen=%d", maxclen);
	printf("\nwmistart=%d", wmistart);
	printf("\nwmimax=%d", wmimax);
	printf("\nwminext=%d", wminext);
}


void testparams()
{
	if ( width<0 || width>IMG_MAX_DIM )
		exitit("width<0 || width>" STR(IMG_MAX_DIM), 1);
	if ( height<0 || height>IMG_MAX_DIM )
		exitit("height<0 || height>" STR(IMG_MAX_DIM), 1);
	if ( bpp<0 || bpp>IMG_MAX_bpp )
		exitit("bpp<0 || bpp>" STR(IMG_MAX_bpp), 1);

	if ( pred<MINpred || pred>MAXpred )
		exitit("pred<MINpred || pred>MAXpred", 1);
	if ( evol<MINevol || evol>MAXevol )
		exitit("evol<MINevol || evol>MAXevol", 1);
	if ( trigger<MINtrigger || trigger>MAXtrigger )
		exitit("trigger<MINtrigger || trigger>MAXtrigger", 1);
	if ( trigger%10 )	/* ! */
		exitit("trigger%10", 1);
	if ( maxclen<=bpp || maxclen>MAXmaxclen )
		exitit("maxclen<=bpp || maxclen>MAXmaxclen", 1);
	if ( wmistart<MINwmistart || wmistart>wmimax )
		exitit("wmistart<MINwmistart || wmistart>wmimax", 1);
	if ( wminext<MINwminext || wminext>MAXwminext )
		exitit("wminext<MINwminext || wminext>MAXwminext", 1);
	if ( 1<<ceil_log_2(wminext) != wminext )	/* ! */
		exitit("1<<ceil_log_2(wminext) != wminext", 1);

	if(decoronly&&!f_compress)
		exitit("-decoronly not allowed for decompression", 1);
}

