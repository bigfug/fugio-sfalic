#ifndef __ENCODECODEWORD_H
#define __ENCODECODEWORD_H

#ifndef NDEBUG
#include "bppmask.h"
#endif

#include "bigendian.h"

/* NOTE fullbytes counts BYTES, but always returns address of the word */
/* so compressedrow and fullbytes are multiplicities of 4, bitsused<32 */
#define ENCODE_START(compressedrow, fullbytes, bitsused)						\
{																				\
	unsigned int bitword=0;				/* word being built */					\
	unsigned int * writeptr=(unsigned int *)(compressedrow+*fullbytes);			\
										/* where to store bitword when full */	\
	unsigned int emptybits;				/* empty bits in bitword*/				\
																				\
	assert(((*fullbytes) % 4)==0);												\
	assert((((BYTE *)NULL-compressedrow) % 4) == 0);							\
																				\
	if(*bitsused)																\
	{																			\
		assert(*bitsused<32);													\
		BE_LOAD_WORD(writeptr, bitword);										\
		assert(!(bitword & bppmask[32-*bitsused]));								\
	}																			\
	emptybits=32-*bitsused;														\
	assert(emptybits!=0);


#define ENCODE(codeword, bits)													\
	{																			\
		assert( bits<=32 );														\
		assert( bits!=0 );														\
		assert( !(codeword & ~bppmask[bits]) );									\
																				\
		if (emptybits>bits) /* fits and leaves space, << and OR */				\
		{																		\
			emptybits-=bits;													\
			bitword|=codeword<<emptybits;										\
		}																		\
		else	/* write required */											\
		{																		\
			const unsigned int fullbits=bits-emptybits;							\
			const unsigned int towrite=bitword|(codeword>>fullbits);			\
																				\
			assert(codeword==(codeword>>0));									\
			emptybits=32-fullbits;												\
			if(fullbits)														\
				bitword=codeword<<emptybits;									\
			else																\
				bitword=0;														\
																				\
			BE_STORE_WORD(writeptr++, towrite);									\
		}																		\
																				\
		assert(emptybits!=0);													\
		assert(emptybits<=32);													\
		assert((bitword&bppmask[emptybits])==0);								\
	}


#define ENCODE_STOP(compressedrow, fullbytes, bitsused)							\
	if(emptybits!=32)															\
		BE_STORE_WORD(writeptr, bitword);										\
																				\
	*fullbytes=(unsigned int)((BYTE*)writeptr-compressedrow);					\
	*bitsused=32-emptybits;														\
																				\
	assert(!*bitsused || !(bitword & bppmask[emptybits]));						\
}


#if 0
void encodecodewords(const thecodeword precompressedrow[], BYTE compressedrow[], 
					 const unsigned int width, 
					 unsigned int * fullbytes, unsigned int * bitsused)
{
	unsigned int i;			

	ENCODE_START(compressedrow, fullbytes, bitsused);

	for(i=0; i<width; i++)
		ENCODE(precompressedrow[i].codeword, precompressedrow[i].codewordlen);

	ENCODE_STOP(compressedrow, fullbytes, bitsused);
}
#endif


#endif
