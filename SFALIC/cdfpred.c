#include "cdftypes.h"
#include "cdfpred.h"
#include "bppmask.h"
#include "assert.h"
#include <memory.h>


/* array for translating distribution U to L for depths up to 8 bpp, initialized by decorelateinit() */
static BYTE xlatU2L[256]; 

/* array for translating distribution L to U for depths up to 8 bpp, initialized by corelateinit() */
static unsigned int xlatL2U[256]; 


/* source code compression ;) */
#define PIXEL_A ((unsigned int)currow[i-1])
#define PIXEL_B ((unsigned int)prevrow[i])
#define PIXEL_C ((unsigned int)prevrow[i-1])


/* single pixel decorrelation */
/*  0  */
static void decorelate_onepixel_0(const PIXEL * const currow, PIXEL * const decorelatedrow, 
		    					  const unsigned int pixelbitmask)
{
	const unsigned int s=*currow; 
		
	if (s<=(pixelbitmask>>1)) 
		*decorelatedrow=s<<1; 
	else 
		*decorelatedrow=((pixelbitmask-s)<<1)+1; 
}


static void decorelate_onepixel_0_8bpp(const BYTE * const currow, BYTE * const decorelatedrow, 
		    						   const unsigned int pixelbitmask)
{
	const unsigned int s=*currow; 
		
	*decorelatedrow=xlatU2L[s];
}


static void decorelate_onepixel_2(const PIXEL * const prevrow, const PIXEL * const currow, 
								  PIXEL * const decorelatedrow, const unsigned int pixelbitmask)
{
	const unsigned int s=(unsigned)( (int)(*currow) - (int)(*prevrow) ) & pixelbitmask; 
		
	if (s<=(pixelbitmask>>1)) 
		*decorelatedrow=s<<1; 
	else 
		*decorelatedrow=((pixelbitmask-s)<<1)+1; 
}


static void decorelate_onepixel_2_8bpp(const BYTE * const prevrow, const BYTE * const currow, 
									   BYTE * const decorelatedrow, const unsigned int pixelbitmask)
{
	const unsigned int s=(unsigned)( (int)(*currow) - (int)(*prevrow) ) & pixelbitmask; 
		
	*decorelatedrow=xlatU2L[s];
}


/* single row decorrelation */


/*  0  */
static void decorelate_0(const PIXEL currow[], const unsigned int rowlen, const int bpp,
						 PIXEL decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int pixelbitmaskshr=pixelbitmask>>1;
	unsigned int i;

	for (i=0; i<rowlen; i++)
	{
		const unsigned int s=currow[i]; 
		
		if (s<=pixelbitmaskshr) 
			decorelatedrow[i]=s<<1; 
		else 
			decorelatedrow[i]=((pixelbitmask-s)<<1)+1; 
	}
}


static void decorelate_0_8bpp(const BYTE currow[], const unsigned int rowlen, const int bpp,
							  BYTE decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int pixelbitmaskshr=pixelbitmask>>1;
	unsigned int i;

	for (i=0; i<rowlen; i++)
	{
		const unsigned int s=currow[i]; 
		
		decorelatedrow[i]=xlatU2L[s];
	}
}


/*  a  */
static void decorelate_1(const PIXEL currow[], const unsigned int rowlen, const int bpp,
						 PIXEL decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int pixelbitmaskshr=pixelbitmask>>1;
	unsigned int i;

	assert(rowlen);	/* 1st (0th) pixel has no left-hand neighbor */
	decorelate_onepixel_0(currow, decorelatedrow, pixelbitmask);
	for (i=1; i<rowlen; i++)
	{
		const unsigned int s=(unsigned)( (int)currow[i] - (int)PIXEL_A ) & pixelbitmask; 
		
		if (s<=pixelbitmaskshr) 
			decorelatedrow[i]=s<<1; 
		else 
			decorelatedrow[i]=((pixelbitmask-s)<<1)+1; 
	}
}


static void decorelate_1_8bpp(const BYTE currow[], const unsigned int rowlen, const int bpp,
							  BYTE decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int pixelbitmaskshr=pixelbitmask>>1;
	unsigned int i;

	assert(rowlen);	
	decorelate_onepixel_0_8bpp(currow, decorelatedrow, pixelbitmask);
	for (i=1; i<rowlen; i++)
	{
		const unsigned int s=(unsigned)( (int)currow[i] - (int)PIXEL_A ) & pixelbitmask; 
		
		decorelatedrow[i]=xlatU2L[s];
	}
}


/*  b  */
static void decorelate_2(const PIXEL prevrow[], const PIXEL currow[], const unsigned int rowlen, const int bpp,
						 PIXEL decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int pixelbitmaskshr=pixelbitmask>>1;
	unsigned int i;

	for (i=0; i<rowlen; i++)
	{
		const unsigned int s=(unsigned)( (int)currow[i] - (int)PIXEL_B ) & pixelbitmask; 
		
		if (s<=pixelbitmaskshr) 
			decorelatedrow[i]=s<<1; 
		else 
			decorelatedrow[i]=((pixelbitmask-s)<<1)+1; 
	}
}


static void decorelate_2_8bpp(const BYTE prevrow[], const BYTE currow[], const unsigned int rowlen, const int bpp,
							  BYTE decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int pixelbitmaskshr=pixelbitmask>>1;
	unsigned int i;

	for (i=0; i<rowlen; i++)
	{
		const unsigned int s=(unsigned)( (int)currow[i] - (int)PIXEL_B ) & pixelbitmask; 
		
		decorelatedrow[i]=xlatU2L[s];
	}
}


/*  c  */
static void decorelate_3(const PIXEL prevrow[], const PIXEL currow[], const unsigned int rowlen, const int bpp,
						 PIXEL decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int pixelbitmaskshr=pixelbitmask>>1;
	unsigned int i;

	assert(rowlen);	
	decorelate_onepixel_2(prevrow, currow, decorelatedrow, pixelbitmask);
	for (i=1; i<rowlen; i++)
	{
		const unsigned int s=(unsigned)( (int)currow[i] - (int)PIXEL_C ) & pixelbitmask; 
		
		if (s<=pixelbitmaskshr) 
			decorelatedrow[i]=s<<1; 
		else 
			decorelatedrow[i]=((pixelbitmask-s)<<1)+1; 
	}
}


static void decorelate_3_8bpp(const BYTE prevrow[], const BYTE currow[], const unsigned int rowlen, const int bpp,
							  BYTE decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int pixelbitmaskshr=pixelbitmask>>1;
	unsigned int i;

	assert(rowlen);	
	decorelate_onepixel_2_8bpp(prevrow, currow, decorelatedrow, pixelbitmask);
	for (i=1; i<rowlen; i++)
	{
		const unsigned int s=(unsigned)( (int)currow[i] - (int)PIXEL_C ) & pixelbitmask; 
		
		decorelatedrow[i]=xlatU2L[s];
	}
}


/*  a+b-c  */
static void decorelate_4(const PIXEL prevrow[], const PIXEL currow[], const unsigned int rowlen, const int bpp,
						 PIXEL decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int pixelbitmaskshr=pixelbitmask>>1;
	unsigned int i;

	assert(rowlen);	
	decorelate_onepixel_2(prevrow, currow, decorelatedrow, pixelbitmask);
	for (i=1; i<rowlen; i++)
	{
		int p=((int)(PIXEL_A+PIXEL_B) - (int)PIXEL_C);

		if (p<0)
			p=0;
		else if ((unsigned)p>pixelbitmask)
			p=pixelbitmask;

		{
			const unsigned int s=(unsigned)( (int)currow[i] - p ) & pixelbitmask; 
			
			if (s<=pixelbitmaskshr) 
				decorelatedrow[i]=s<<1; 
			else 
				decorelatedrow[i]=((pixelbitmask-s)<<1)+1; 
		}
	}
}


static void decorelate_4_8bpp(const BYTE prevrow[], const BYTE currow[], const unsigned int rowlen, const int bpp,
						      BYTE decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int pixelbitmaskshr=pixelbitmask>>1;
	unsigned int i;

	assert(rowlen);	
	decorelate_onepixel_2_8bpp(prevrow, currow, decorelatedrow, pixelbitmask);
	for (i=1; i<rowlen; i++)
	{
		int p=((int)(PIXEL_A+PIXEL_B) - (int)PIXEL_C);

		if (p<0)
			p=0;
		else if ((unsigned)p>pixelbitmask)
			p=pixelbitmask;

		{
			const unsigned int s=(unsigned)( (int)currow[i] - p ) & pixelbitmask; 
			
			decorelatedrow[i]=xlatU2L[s];
		}
	}
}


/*  a+(b-c)/2  */
static void decorelate_5(const PIXEL prevrow[], const PIXEL currow[], const unsigned int rowlen, const int bpp,
						 PIXEL decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int pixelbitmaskshr=pixelbitmask>>1;
	unsigned int i;

	assert(rowlen);	
	decorelate_onepixel_2(prevrow, currow, decorelatedrow, pixelbitmask);
	for (i=1; i<rowlen; i++)
	{
		int p=( (int)PIXEL_A + (((int)PIXEL_B-(int)PIXEL_C)>>1) ) ;

		if (p<0)
			p=0;
		else if ((unsigned)p>pixelbitmask)
			p=pixelbitmask;

		{
			const unsigned int s=(unsigned)( (int)currow[i] - p ) & pixelbitmask; 
			
			if (s<=pixelbitmaskshr) 
				decorelatedrow[i]=s<<1; 
			else 
				decorelatedrow[i]=((pixelbitmask-s)<<1)+1; 
		}
	}
}


static void decorelate_5_8bpp(const BYTE prevrow[], const BYTE currow[], const unsigned int rowlen, const int bpp,
							  BYTE decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int pixelbitmaskshr=pixelbitmask>>1;
	unsigned int i;

	assert(rowlen);	
	decorelate_onepixel_2_8bpp(prevrow, currow, decorelatedrow, pixelbitmask);
	for (i=1; i<rowlen; i++)
	{
		int p=( (int)PIXEL_A + (((int)PIXEL_B-(int)PIXEL_C)>>1) ) ;

		if (p<0)
			p=0;
		else if ((unsigned)p>pixelbitmask)
			p=pixelbitmask;

		{
			const unsigned int s=(unsigned)( (int)currow[i] - p ) & pixelbitmask; 
			
			decorelatedrow[i]=xlatU2L[s];
		}
	}
}


/*  b+(a-c)/2  */
static void decorelate_6(const PIXEL prevrow[], const PIXEL currow[], const unsigned int rowlen, const int bpp,
						 PIXEL decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int pixelbitmaskshr=pixelbitmask>>1;
	unsigned int i;

	assert(rowlen);	
	decorelate_onepixel_2(prevrow, currow, decorelatedrow, pixelbitmask);
	for (i=1; i<rowlen; i++)
	{
		int p=( (int)PIXEL_B + (((int)PIXEL_A-(int)PIXEL_C)>>1) );

		if (p<0)
			p=0;
		else if ((unsigned)p>pixelbitmask)
			p=pixelbitmask;

		{
			const unsigned int s=(unsigned)( (int)currow[i] - p ) & pixelbitmask; 
			
			if (s<=pixelbitmaskshr) 
				decorelatedrow[i]=s<<1; 
			else 
				decorelatedrow[i]=((pixelbitmask-s)<<1)+1; 
		}
	}
}


static void decorelate_6_8bpp(const BYTE prevrow[], const BYTE currow[], const unsigned int rowlen, const int bpp,
							  BYTE decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int pixelbitmaskshr=pixelbitmask>>1;
	unsigned int i;

	assert(rowlen);	
	decorelate_onepixel_2_8bpp(prevrow, currow, decorelatedrow, pixelbitmask);
	for (i=1; i<rowlen; i++)
	{
		int p=( (int)PIXEL_B + (((int)PIXEL_A-(int)PIXEL_C)>>1) );

		if (p<0)
			p=0;
		else if ((unsigned)p>pixelbitmask)
			p=pixelbitmask;

		{
			const unsigned int s=(unsigned)( (int)currow[i] - p ) & pixelbitmask; 
			
			decorelatedrow[i]=xlatU2L[s];
		}
	}
}


/*  (a+b)/2  */
static void decorelate_7(const PIXEL prevrow[], const PIXEL currow[], const unsigned int rowlen, const int bpp,
						 PIXEL decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int pixelbitmaskshr=pixelbitmask>>1;
	unsigned int i;

	assert(rowlen);	
	decorelate_onepixel_2(prevrow, currow, decorelatedrow, pixelbitmask);
	for (i=1; i<rowlen; i++)
	{
		const unsigned int s=(unsigned)( (int)currow[i] - (int)((PIXEL_A+PIXEL_B)>>1) ) & pixelbitmask; 
		
		if (s<=pixelbitmaskshr) 
			decorelatedrow[i]=s<<1; 
		else 
			decorelatedrow[i]=((pixelbitmask-s)<<1)+1; 
	}
}


static void decorelate_7_8bpp(const BYTE prevrow[], const BYTE currow[], const unsigned int rowlen, const int bpp,
							  BYTE decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int pixelbitmaskshr=pixelbitmask>>1;
	unsigned int i;

	assert(rowlen);
	decorelate_onepixel_2_8bpp(prevrow, currow, decorelatedrow, pixelbitmask);
	for (i=1; i<rowlen; i++)
	{
		const unsigned int s=(unsigned)( (int)currow[i] - (int)((PIXEL_A+PIXEL_B)>>1) ) & pixelbitmask; 
		
		decorelatedrow[i]=xlatU2L[s];
	}
}


/*  .75a+.75b-.5c  */
static void decorelate_8(const PIXEL prevrow[], const PIXEL currow[], const unsigned int rowlen, const int bpp,
						 PIXEL decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int pixelbitmaskshr=pixelbitmask>>1;
	unsigned int i;

	assert(rowlen);	
	decorelate_onepixel_2(prevrow, currow, decorelatedrow, pixelbitmask);
	for (i=1; i<rowlen; i++)
	{
		int p=( (int)(3*(PIXEL_A+PIXEL_B)) - (int)(PIXEL_C<<1) ) >>2  ;

		if (p<0)
			p=0;
		else if ((unsigned)p>pixelbitmask)
			p=pixelbitmask;

		{
			const unsigned int s=(unsigned)( (int)currow[i] - p ) & pixelbitmask; 
			
			if (s<=pixelbitmaskshr) 
				decorelatedrow[i]=s<<1; 
			else 
				decorelatedrow[i]=((pixelbitmask-s)<<1)+1; 
		}
	}
}


static void decorelate_8_8bpp(const BYTE prevrow[], const BYTE currow[], const unsigned int rowlen, const int bpp,
							  BYTE decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int pixelbitmaskshr=pixelbitmask>>1;
	unsigned int i;

	assert(rowlen);	
	decorelate_onepixel_2_8bpp(prevrow, currow, decorelatedrow, pixelbitmask);
	for (i=1; i<rowlen; i++)
	{
		int p=( (int)(3*(PIXEL_A+PIXEL_B)) - (int)(PIXEL_C<<1) ) >>2  ;

		if (p<0)
			p=0;
		else if ((unsigned)p>pixelbitmask)
			p=pixelbitmask;

		{
			const unsigned int s=(unsigned)( (int)currow[i] - p ) & pixelbitmask; 
			
			decorelatedrow[i]=xlatU2L[s];
		}
	}
}


/* ********************************************************************************************************************* */
/* correlate (no de-) */


static void corelate_onepixel_0(PIXEL * const currow, 
								const PIXEL * const decorelatedrow, const unsigned int pixelbitmask)
{
	const unsigned int s=*decorelatedrow;
	
	if (s & 0x01)
		*currow=(PIXEL)(pixelbitmask-(s>>1));
	else 
		*currow=(PIXEL)(s>>1);
}


static void corelate_onepixel_0_8bpp(BYTE * const currow, 
								const BYTE * const decorelatedrow, const unsigned int pixelbitmask)
{
	const unsigned int s=xlatL2U[*decorelatedrow];

	*currow=s;
}


static void corelate_onepixel_2(const PIXEL * const prevrow, PIXEL * const currow, 
								const PIXEL * const decorelatedrow, const unsigned int pixelbitmask)
{
	const unsigned int s=*decorelatedrow;
	
	if (s & 0x01)
		*currow=(PIXEL)( ((pixelbitmask-(s>>1)) + *prevrow) & pixelbitmask);
	else 
		*currow=(PIXEL)( ((s>>1) + *prevrow) & pixelbitmask);
}


static void corelate_onepixel_2_8bpp(const BYTE * const prevrow, BYTE * const currow, 
								const BYTE * const decorelatedrow, const unsigned int pixelbitmask)
{
	const unsigned int s=xlatL2U[*decorelatedrow];
	
	*currow=(s + *prevrow) & pixelbitmask;
}


static void corelate_0(PIXEL currow[], const unsigned int rowlen, const int bpp,
						 const PIXEL decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	unsigned int i;

	assert(rowlen);	
	for (i=0; i<rowlen; i++)
	{
		const unsigned int s=decorelatedrow[i];

		if (s & 0x01)
			currow[i]=(PIXEL)(pixelbitmask-(s>>1));
		else 
			currow[i]=(PIXEL)(s>>1);
	}
}


static void corelate_0_8bpp(BYTE currow[], const unsigned int rowlen, const int bpp,
						 const BYTE decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	unsigned int i;

	assert(rowlen);
	for (i=0; i<rowlen; i++)
	{
		const unsigned int s=xlatL2U[decorelatedrow[i]];

		currow[i]=s;
	}
}


static void corelate_1(PIXEL currow[], const unsigned int rowlen, const int bpp,
						 const PIXEL decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	unsigned int i;
	unsigned int A;

	assert(rowlen);
	corelate_onepixel_0(currow, decorelatedrow, pixelbitmask);
	A=currow[0];
	for (i=1; i<rowlen; i++)
	{
		const unsigned int p=(int)A;
		const unsigned int s=decorelatedrow[i];

		if (s & 0x01)
			currow[i]=A=( ((pixelbitmask-(s>>1)) + p) & pixelbitmask);
		else 
			currow[i]=A=( ((s>>1) + p) & pixelbitmask);/**/
	}
}


static void corelate_1_8bpp(BYTE currow[], const unsigned int rowlen, const int bpp,
						 const BYTE decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	unsigned int i;
	unsigned int A;

	assert(rowlen);
	corelate_onepixel_0_8bpp(currow, decorelatedrow, pixelbitmask);
	A=currow[0];
	for (i=1; i<rowlen; i++)
	{
		const unsigned int p=(int)A;
		const unsigned int s=xlatL2U[decorelatedrow[i]];

		A=(s + p) & pixelbitmask;
		currow[i]=A;
	}
}


static void corelate_2(const PIXEL prevrow[], PIXEL currow[], const unsigned int rowlen, const int bpp,
						 const PIXEL decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	unsigned int i;

	assert(rowlen);
	for (i=0; i<rowlen; i++)
	{
		const unsigned int p=(int)PIXEL_B;
		const unsigned int s=decorelatedrow[i];

		if (s & 0x01)
			currow[i]=(PIXEL)( ((pixelbitmask-(s>>1)) + p) & pixelbitmask);
		else 
			currow[i]=(PIXEL)( ((s>>1) + p) & pixelbitmask);
	}
}


static void corelate_2_8bpp(const BYTE prevrow[], BYTE currow[], const unsigned int rowlen, const int bpp,
						 const BYTE decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	unsigned int i;

	assert(rowlen);
	for (i=0; i<rowlen; i++)
	{
		const unsigned int p=(int)PIXEL_B;
		const unsigned int s=xlatL2U[decorelatedrow[i]];

		currow[i]=(s + p) & pixelbitmask;
	}
}


static void corelate_3(const PIXEL prevrow[], PIXEL currow[], const unsigned int rowlen, const int bpp,
						 const PIXEL decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	unsigned int i;

	assert(rowlen);
	corelate_onepixel_2(prevrow, currow, decorelatedrow, pixelbitmask);
	for (i=1; i<rowlen; i++)
	{
		const unsigned int p=(int)PIXEL_C;
		const unsigned int s=decorelatedrow[i];

		if (s & 0x01)
			currow[i]=(PIXEL)( ((pixelbitmask-(s>>1)) + p) & pixelbitmask);
		else 
			currow[i]=(PIXEL)( ((s>>1) + p) & pixelbitmask);
	}
}


static void corelate_3_8bpp(const BYTE prevrow[], BYTE currow[], const unsigned int rowlen, const int bpp,
						 const BYTE decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	unsigned int i;

	assert(rowlen);
	corelate_onepixel_2_8bpp(prevrow, currow, decorelatedrow, pixelbitmask);
	for (i=1; i<rowlen; i++)
	{
		const unsigned int p=(int)PIXEL_C;
		const unsigned int s=xlatL2U[decorelatedrow[i]];

		currow[i]=(s + p) & pixelbitmask;
	}
}


static void corelate_4(const PIXEL prevrow[], PIXEL currow[], const unsigned int rowlen, const int bpp,
						 const PIXEL decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	unsigned int i;
	unsigned int A;

	assert(rowlen);
	corelate_onepixel_2(prevrow, currow, decorelatedrow, pixelbitmask);
	A=currow[0];
	for (i=1; i<rowlen; i++)
	{
		int p=((int)(A+PIXEL_B) - (int)PIXEL_C);

		if (p<0)
			p=0;
		else if ((unsigned)p>pixelbitmask)
			p=pixelbitmask;

		{
			const unsigned int s=decorelatedrow[i];

			if (s & 0x01)
				currow[i]=A=( ((pixelbitmask-(s>>1)) + p) & pixelbitmask);
			else 
				currow[i]=A=( ((s>>1) + p) & pixelbitmask);/**/
		}
	}
}


static void corelate_4_8bpp(const BYTE prevrow[], BYTE currow[], const unsigned int rowlen, const int bpp,
						 const BYTE decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int negpixelbitmask=~pixelbitmask;
	unsigned int i;
	unsigned int A;

	assert(rowlen);
	corelate_onepixel_2_8bpp(prevrow, currow, decorelatedrow, pixelbitmask);
	A=currow[0];
	for (i=1; i<rowlen; i++)
	{
		const int p=((int)(A+PIXEL_B) - (int)PIXEL_C);
		const unsigned int s=xlatL2U[decorelatedrow[i]];

		if (! (p&negpixelbitmask))
			A=(s + (unsigned)p) & pixelbitmask;
		else if (p<0)
			A=s;
		else
			A=(s + pixelbitmask) & pixelbitmask;

		currow[i]=A;
	}
}


static void corelate_5(const PIXEL prevrow[], PIXEL currow[], const unsigned int rowlen, const int bpp,
						 const PIXEL decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	unsigned int i;
	unsigned int A;

	assert(rowlen);
	corelate_onepixel_2(prevrow, currow, decorelatedrow, pixelbitmask);
	A=currow[0];
	for (i=1; i<rowlen; i++)
	{
		int p=( (int)A + (((int)PIXEL_B-(int)PIXEL_C)>>1) ) ;

		if (p<0)
			p=0;
		else if ((unsigned)p>pixelbitmask)
			p=pixelbitmask;

		{
			const unsigned int s=decorelatedrow[i];

			if (s & 0x01)
				currow[i]=A=( ((pixelbitmask-(s>>1)) + p) & pixelbitmask);
			else 
				currow[i]=A=( ((s>>1) + p) & pixelbitmask);/**/
		}
	}
}


static void corelate_5_8bpp(const BYTE prevrow[], BYTE currow[], const unsigned int rowlen, const int bpp,
						 const BYTE decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int negpixelbitmask=~pixelbitmask;
	unsigned int i;
	unsigned int A;

	assert(rowlen);
	corelate_onepixel_2_8bpp(prevrow, currow, decorelatedrow, pixelbitmask);
	A=currow[0];
	for (i=1; i<rowlen; i++)
	{
		const int p=( (int)A + (((int)PIXEL_B-(int)PIXEL_C)>>1) ) ;
		const unsigned int s=xlatL2U[decorelatedrow[i]];

		if (! (p&negpixelbitmask))
			A=(s + (unsigned)p) & pixelbitmask;
		else if (p<0)
			A=s;
		else
			A=(s + pixelbitmask) & pixelbitmask;

		currow[i]=A;
	}
}


static void corelate_6(const PIXEL prevrow[], PIXEL currow[], const unsigned int rowlen, const int bpp,
						 const PIXEL decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	unsigned int i;
	unsigned int A;

	assert(rowlen);
	corelate_onepixel_2(prevrow, currow, decorelatedrow, pixelbitmask);
	A=currow[0];
	for (i=1; i<rowlen; i++)
	{
		int p=( (int)PIXEL_B + (((int)A-(int)PIXEL_C)>>1) );

		if (p<0)
			p=0;
		else if ((unsigned)p>pixelbitmask)
			p=pixelbitmask;

		{
			const unsigned int s=decorelatedrow[i];

			if (s & 0x01)
				currow[i]=A=( ((pixelbitmask-(s>>1)) + p) & pixelbitmask);
			else 
				currow[i]=A=( ((s>>1) + p) & pixelbitmask);/**/
		}
	}
}


static void corelate_6_8bpp(const BYTE prevrow[], BYTE currow[], const unsigned int rowlen, const int bpp,
						 const BYTE decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int negpixelbitmask=~pixelbitmask;
	unsigned int i;
	unsigned int A;

	assert(rowlen);
	corelate_onepixel_2_8bpp(prevrow, currow, decorelatedrow, pixelbitmask);
	A=currow[0];
	for (i=1; i<rowlen; i++)
	{
		const int p=( (int)PIXEL_B + (((int)A-(int)PIXEL_C)>>1) );
		const unsigned int s=xlatL2U[decorelatedrow[i]];

		if (! (p&negpixelbitmask))
			A=(s + (unsigned)p) & pixelbitmask;
		else if (p<0)
			A=s;
		else
			A=(s + pixelbitmask) & pixelbitmask;

		currow[i]=A;
	}
}


static void corelate_7(const PIXEL prevrow[], PIXEL currow[], const unsigned int rowlen, const int bpp,
						 const PIXEL decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	unsigned int i;
	unsigned int A;

	assert(rowlen);
	corelate_onepixel_2(prevrow, currow, decorelatedrow, pixelbitmask);
	A=currow[0];
	for (i=1; i<rowlen; i++)
	{
		const unsigned int p=(int)((A+PIXEL_B)>>1);
		const unsigned int s=decorelatedrow[i];

		if (s & 0x01)
			currow[i]=A=( ((pixelbitmask-(s>>1)) + p) & pixelbitmask);
		else 
			currow[i]=A=( ((s>>1) + p) & pixelbitmask);/**/
	}
}


static void corelate_7_8bpp(const BYTE prevrow[], BYTE currow[], const unsigned int rowlen, const int bpp,
						 const BYTE decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	unsigned int i;
	unsigned int A;

	assert(rowlen);
	corelate_onepixel_2_8bpp(prevrow, currow, decorelatedrow, pixelbitmask);
	A=currow[0];
	for (i=1; i<rowlen; i++)
	{
		const unsigned int p=(int)((A+PIXEL_B)>>1);
		const unsigned int s=xlatL2U[decorelatedrow[i]];

		A=(s + p) & pixelbitmask;
		currow[i]=A;
	}
}


static void corelate_8(const PIXEL prevrow[], PIXEL currow[], const unsigned int rowlen, const int bpp,
						 const PIXEL decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	unsigned int i;
	unsigned int A;

	assert(rowlen);
	corelate_onepixel_2(prevrow, currow, decorelatedrow, pixelbitmask);
	A=currow[0];
	for (i=1; i<rowlen; i++)
	{
		int p=( (int)(3*(A+PIXEL_B)) - (int)(PIXEL_C<<1) ) >>2  ;

		if (p<0)
			p=0;
		else if ((unsigned)p>pixelbitmask)
			p=pixelbitmask;

		{
			const unsigned int s=decorelatedrow[i];

			if (s & 0x01)
				currow[i]=A=( ((pixelbitmask-(s>>1)) + p) & pixelbitmask);
			else 
				currow[i]=A=( ((s>>1) + p) & pixelbitmask);/**/
		}
	}
}


static void corelate_8_8bpp(const BYTE prevrow[], BYTE currow[], const unsigned int rowlen, const int bpp,
						 const BYTE decorelatedrow[])
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int negpixelbitmask=~pixelbitmask;
	unsigned int i;
	unsigned int A;

	assert(rowlen);
	corelate_onepixel_2_8bpp(prevrow, currow, decorelatedrow, pixelbitmask);
	A=currow[0];
	for (i=1; i<rowlen; i++)
	{
		const int p=( (int)(3*(A+PIXEL_B)) - (int)(PIXEL_C<<1) ) >>2  ;
		const unsigned int s=xlatL2U[decorelatedrow[i]];

		if (! (p&negpixelbitmask))
			A=(s + (unsigned)p) & pixelbitmask;
		else if (p<0)
			A=s;
		else
			A=(s + pixelbitmask) & pixelbitmask;

		currow[i]=A;
	}
}


void decorelaterow(const PIXEL *prevrow, const PIXEL *currow, int row, int rowlen, int bpp,
				   int pred, PIXEL *decorelatedrow)
{
	assert(pred<=MAXpred && pred>=MINpred);
	assert(rowlen>0);
	assert(row>=0);

	if (pred==-1) /* copy */
	{
		memcpy(decorelatedrow, currow, sizeof(PIXEL)*rowlen);
		return;
	}

	if (row==0 && pred!=0) /* for 1st (0th) row instead of each predictor, but pred0, use the pred1 */
		pred=1;

	switch (pred)
	{
	case 0: decorelate_0(currow, rowlen, bpp, decorelatedrow); return;
	case 1: decorelate_1(currow, rowlen, bpp, decorelatedrow); return;
	case 2: decorelate_2(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 3: decorelate_3(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 4: decorelate_4(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 5: decorelate_5(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 6: decorelate_6(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 7: decorelate_7(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 8: decorelate_8(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	default: assert(0);
	}
}


void decorelaterow8bpp(const BYTE *prevrow, const BYTE *currow, int row, int rowlen, int bpp,
				       int pred, BYTE *decorelatedrow)
{
	assert(bpp<=8);
	assert(pred<=MAXpred && pred>=MINpred);
	assert(rowlen>0);
	assert(row>=0);

	if (pred==-1) 
	{
		memcpy(decorelatedrow, currow, rowlen);
		return;
	}

	if (row==0 && pred!=0)
		pred=1;

	switch (pred)
	{
	case 0: decorelate_0_8bpp(currow, rowlen, bpp, decorelatedrow); return;
	case 1: decorelate_1_8bpp(currow, rowlen, bpp, decorelatedrow); return;
	case 2: decorelate_2_8bpp(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 3: decorelate_3_8bpp(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 4: decorelate_4_8bpp(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 5: decorelate_5_8bpp(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 6: decorelate_6_8bpp(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 7: decorelate_7_8bpp(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 8: decorelate_8_8bpp(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	default: assert(0);
	}
}


void decorelateinit8bpp(int bpp)
{
	const unsigned int pixelbitmask=bppmask[bpp];
	const unsigned int pixelbitmaskshr=pixelbitmask>>1;
	unsigned int s;

	assert(bpp<=8);

	for (s=0; s<=pixelbitmask; s++)
		if (s<=pixelbitmaskshr) 
			xlatU2L[s]=s<<1; 
		else 
			xlatU2L[s]=((pixelbitmask-s)<<1)+1; 
}


void corelaterow(const PIXEL *prevrow, PIXEL *currow, int row, int rowlen, int bpp,
				   int pred, const PIXEL *decorelatedrow)
{
	assert(pred<=MAXpred && pred>=MINpred);
	assert(rowlen>0);
	assert(row>=0);

	if (pred==-1)
	{
		memcpy(currow, decorelatedrow, sizeof(PIXEL)*rowlen);
		return;
	}

	if (row==0 && pred!=0)
		pred=1;

	switch (pred)
	{
	case 0: corelate_0(currow, rowlen, bpp, decorelatedrow); return;
	case 1: corelate_1(currow, rowlen, bpp, decorelatedrow); return;
	case 2: corelate_2(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 3: corelate_3(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 4: corelate_4(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 5: corelate_5(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 6: corelate_6(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 7: corelate_7(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 8: corelate_8(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	default: assert(0);
	}
}


void corelaterow8bpp(const BYTE *prevrow, BYTE *currow, int row, int rowlen, int bpp,
				   int pred, const BYTE *decorelatedrow)
{
	assert(pred<=MAXpred && pred>=MINpred);
	assert(rowlen>0);
	assert(row>=0);

	if (pred==-1)
	{
		memcpy(currow, decorelatedrow, rowlen);
		return;
	}

	if (row==0 && pred!=0)
		pred=1;

	switch (pred)
	{
	case 0: corelate_0_8bpp(currow, rowlen, bpp, decorelatedrow); return;
	case 1: corelate_1_8bpp(currow, rowlen, bpp, decorelatedrow); return;
	case 2: corelate_2_8bpp(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 3: corelate_3_8bpp(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 4: corelate_4_8bpp(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 5: corelate_5_8bpp(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 6: corelate_6_8bpp(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 7: corelate_7_8bpp(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	case 8: corelate_8_8bpp(prevrow, currow, rowlen, bpp, decorelatedrow); return;
	default: assert(0);
	}
}


void corelateinit8bpp(int bpp)
{
	const unsigned int pixelbitmask=bppmask[bpp];
	unsigned int s;

	assert(bpp<=8);

	for (s=0; s<=pixelbitmask; s++)
		if (s & 0x01) 
			xlatL2U[s]=pixelbitmask-(s>>1); 
		else 
			xlatL2U[s]=(s>>1); 
}
