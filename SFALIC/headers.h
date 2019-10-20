#ifndef __HEADERS_H
#define __HEADERS_H

/* functions processing headers of files in formats PGMP5 and CDF */

#include <stdio.h>


/* read PGMP5 image file header, fill: width, height and bpp */
/* set file position to the first byte of the first image pixel */
void readPGMP5header(FILE *f);

/* store PGMP5 header using width, height and bpp */
/* do not close f */
void writePGMP5header(FILE *f);

/* store the CDF header using params from taskparams */
/* do not close the f */
void writeCDFheader(FILE *f);

/* read CDF header, fill taskparams vars */
/* set file position to the first byte of the first compressed image pixel */
void readCDFheader(FILE *f);


#endif
