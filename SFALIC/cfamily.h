#ifndef __CFAMILY_H
#define __CFAMILY_H

#include <stdio.h>
#include "cdftypes.h"

/* code family routines, sub-module for cdfstat */

/* init module */
void familyinit(int bpp, int limit);

/* args: symbol to encode:n, code number:l, addr where to store the codeword and its length */
void GolombCoding(const PIXEL n, const unsigned int l, 
				  unsigned int * const codeword, unsigned int * const codewordlen);

/* args: symbol:n, code number:l */
unsigned int GolombCodeLen(const PIXEL n, const unsigned int l);

/* args: code number:l, int containing the codeword as the msb:bits, address to store the codeword length */
/* returns decoded codeword (does not remove codeword from bits) */
unsigned int GolombDecoding(const unsigned int l, 
							const unsigned int bits, unsigned int * const codewordlen);

#endif

