#include "decodecodeword.h"
#include <assert.h>


void bitinstatusinit(struct bitinstatus * bs)
{
	assert((((unsigned int)(bs->rdptr)) % 4)==0);

	BE_LOAD_WORD(bs->rdptr, bs->bits);
	bs->rdptr+=4;
	BE_LOAD_WORD(bs->rdptr, bs->nextword);
	bs->rdptr+=4;

	bs->innextword=32;
}

