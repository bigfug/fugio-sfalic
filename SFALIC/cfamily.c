
/* code family (without adjusted binary codes, for depths up to 16 bpp, codeword length limit up to 31 ) R. Starosolski, W. Skarbek: Modified Golomb-Rice Codes for Lossless Compression of Medical Images", Proceedings of International Conference on E-health in Common Europe, pp. 423-37, Krakow, 2003 */

#include "cfamily.h"
#include "bppmask.h"
#include "ceillog2.h"

static const unsigned int bitat[32] ={
							0x00000001,0x00000002,0x00000004,0x00000008,
							0x00000010,0x00000020,0x00000040,0x00000080,
							0x00000100,0x00000200,0x00000400,0x00000800,
							0x00001000,0x00002000,0x00004000,0x00008000,
							0x00010000,0x00020000,0x00040000,0x00080000,
							0x00100000,0x00200000,0x00400000,0x00800000,
							0x01000000,0x02000000,0x04000000,0x08000000,
							0x10000000,0x20000000,0x40000000,0x80000000 /* [31]*/
							};

static unsigned int nGRcodewords[MAXNUMCODES];		/* indexed by code number, contains number of unmodofied GR codewords in the code */
static unsigned int notGRcwlen[MAXNUMCODES];		/* indexed by code number, contains codeword length of the not-GR codeword */
static unsigned int notGRprefixmask[MAXNUMCODES];	/* indexed by code number, contains mask to determine if the codeword is GR or not-GR */
static unsigned int notGRsuffixlen[MAXNUMCODES];	/* indexed by code number, contains suffix length of the not-GR codeword */

static const BYTE lzeroes[256]={	/* number of leading zeroes in the byte, used by cntlzeroes(uint)*/
		8,7,6,6,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
		2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
		};


void GolombCoding(const PIXEL n, const unsigned int l, 
				  unsigned int * const codeword, unsigned int * const codewordlen)
{
	if(n<nGRcodewords[l])
	{
		(*codeword) = bitat[l] | n & bppmask[l]; 
		(*codewordlen) = (n>>l)+l+1;
	}
	else
	{
		(*codeword) = n-nGRcodewords[l];
		(*codewordlen) = notGRcwlen[l];
	}
}


unsigned int GolombCodeLen(const PIXEL n, const unsigned int l)
{
	if(n<nGRcodewords[l])
		return (n >> l)+1+l;
	else
		return  notGRcwlen[l];
}


/* count leading zeroes */
static unsigned int cntlzeroes(const unsigned int bits)
{
	if (bits & 0xff800000)
		return lzeroes[bits>>24];
	else if (bits & 0xffff8000)
		return 8+lzeroes[(bits>>16)&0x000000ff];
	else if (bits & 0xffffff80)
		return 16+lzeroes[(bits>>8)&0x000000ff];
	else  
		return 24+lzeroes[(bits)&0x000000ff];
}


unsigned int GolombDecoding(const unsigned int l, 
							const unsigned int bits, unsigned int * const codewordlen)
{
	if(bits > notGRprefixmask[l])
	{/*GR*/
		const unsigned int zeroprefix=cntlzeroes(bits);		/* leading zeroes in codeword */
		const unsigned int cwlen=zeroprefix+1+l;			/* codeword length */
		(*codewordlen) = cwlen;
		return (zeroprefix<<l) | ( ( bits>>(32-cwlen) ) & bppmask[l] );
	}
	else
	{/* not-GR */
		const unsigned int cwlen=notGRcwlen[l];
		(*codewordlen) = cwlen;
		return nGRcodewords[l] + ( (bits)>>(32-cwlen) & bppmask[notGRsuffixlen[l]] );
	}
}


#include <assert.h>


void familyinit(int bpp, int limit)
{
	int l;

	assert(limit<=32);
	assert(bpp<=MAXNUMCODES);
	assert(limit>bpp);

	for (l=0; l<bpp; l++)	/* fill arrays indexed by code number */
	{
		int altprefixlen, altcodewords;

		altprefixlen = limit-bpp;
		if (altprefixlen > (int)(bppmask[bpp-l]))
			altprefixlen=bppmask[bpp-l];

	    altcodewords=bppmask[bpp]+1-(altprefixlen<<l);

		nGRcodewords[l]=(altprefixlen<<l);
		notGRcwlen[l]=altprefixlen+ceil_log_2(altcodewords);
		notGRprefixmask[l]=bppmask[32-altprefixlen];	/* needed for decoding only */
		notGRsuffixlen[l]=ceil_log_2(altcodewords);		/* needed for decoding only */
	}

	return;
}

