#ifndef __DECODECODEWORD_H
#define __DECODECODEWORD_H

#include "bigendian.h"
#include <assert.h>

struct bitinstatus	/* bit input status structure */
{
	unsigned bits;			/* always contains next 32 bits from input*/
	unsigned innextword;	/* how many bits in nextword are not in bits field */
	unsigned nextword;		/* cache for next 32 bits to be inserted into bits field */
	BYTE * rdptr;			/* pointer to word to be inserted to nextword */
};


/* init bitinstatus before first use of it */
/* bs has to contain proper rdptr, bitinstatusinit fills remaining fields */
void bitinstatusinit(struct bitinstatus * bs);


/* declare bit input status local variables and init it using  *bs */
#define DECODE_START(bs)								\
{														\
	unsigned int bits=bs->bits;							\
	unsigned int innextword=bs->innextword;				\
	unsigned int nextword=bs->nextword;					\
	unsigned int *rdptr=(unsigned int *)bs->rdptr;		\
	assert((((unsigned int)(bs->rdptr)) % 4)==0);


/* remove from input ile bits, update bits to contain 32 valid bits */
#define DECODE_EATBITS(ile)								\
{														\
	assert (ile);										\
	assert (ile<32 /*ile<=32*/);						\
														\
	/*if (ile<32)*/										\
		bits <<= ile;									\
	/*else*/											\
		/*bits = 0;*/									\
	if (ile <= innextword)								\
	{													\
		innextword -= ile;								\
		bits |= (nextword >> innextword);				\
		if (!innextword)								\
		{												\
			BE_LOAD_WORD(rdptr, nextword);				\
			rdptr++;									\
			innextword = 32;							\
		}												\
	}													\
	else												\
	{													\
		const unsigned int diff=ile-innextword;			\
		bits |= (nextword << diff);						\
		BE_LOAD_WORD(rdptr, nextword);					\
		rdptr++;										\
		innextword = 32-diff;							\
		bits |= (nextword >> innextword);				\
	}													\
}


/* store bit input status in *bs */
#define DECODE_STOP(bs)									\
	bs->bits=bits;										\
	bs->innextword=innextword;							\
	bs->nextword=nextword;								\
	bs->rdptr=(BYTE *)rdptr;							\
}


#endif
