#include "headers.h"
#include "taskparams.h"
#include "bigendian.h"
#include "exitit.h"
#include "ceillog2.h"


void readPGMP5header(FILE *f)
{
	int levelmax;
	if (fscanf(f, "P5%d %d %d", &width, &height, &levelmax) != 3)
		exitit("error reading PGMP5 header", 3);

	if (!(width>0 && height>0 && levelmax>0 && levelmax<=0xFFFF))
		exitit("error in PGMP5 header", 3);

	bpp=ceil_log_2(levelmax+1);

	while (fgetc(f)!=0x0a)
		if (feof(f))
			exitit("error reading PGMP5 header", 3);
}


void writePGMP5header(FILE *f)
{
	int levelmax=((int)0xffff >> (16-bpp));

	if (EOF == fprintf(f, "P5%c%d %d%c%d%c", 0x0A, width, height, 0x0A, levelmax, 0x0A))
		exitit("error writting PGMP5 header", 3);
}


void writeCDFheader(FILE *f)
{
	int fres; /* wynik operacji plikowej */

	fputs("CDF", f);
	fputc(FILEFORMATVERSION, f);
	BEwrite2Bytes(width, f);
	BEwrite2Bytes(height, f);
	fputc(bpp, f);
	fputc(pred, f);

	fputc(evol, f);
	fputc(trigger/10, f);	/* ! */
	fputc(maxclen, f);
	fputc(wmistart, f);
	fputc(wmimax, f);

	fres= fputc(ceil_log_2(wminext), f);	/* ! */

	if (fres==EOF)
		exitit("error writting CDF header", 3);
}


void readCDFheader(FILE *f)
{
	int fres; /* file operation result */

	if (fgetc(f) != 'C' || fgetc(f) != 'D' || fgetc(f) != 'F')
		exitit("bad CDF header", 3);
		
	if (fgetc(f) != FILEFORMATVERSION)
		exitit("bad CDF header version", 3);

	width=BEread2Bytes(f);
	height=BEread2Bytes(f);
	bpp=fgetc(f);

	pred=fgetc(f);
	pred=(int)((i8)pred);	/* pred may be negative ! */

	evol=fgetc(f);

	trigger=fgetc(f)*10;	/* ! */
	maxclen=fgetc(f);
	wmistart=fgetc(f);
	wmimax=fgetc(f);

	fres= wminext=1<<fgetc(f);	/* ! */

	if (fres==EOF)
		exitit("error reading CDF header", 3);
}

