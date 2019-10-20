#ifndef __BIGENDIAN_H
#define __BIGENDIAN_H

/* Endian-dependant functions; checking of the compiler settings and of some other assumptions */

#include "cdftypes.h"
#include <stdio.h>


/* checking of the compiler settings and of some other assumptions */
/* for the whole program, not the bigendian module only */
/* returns 0-ok, >=1 error */
int CheckAssumptions();

/* BE_MACHINE macro should be defined for BigEndian CPU (actually not obligatory) */
/* BE_MACHINE _must_not_ be defined for LittleEndian CPU, checked in CheckAssumptions() */

/* #define BE_MACHINE */

/* BE_STORE_WORD stores the towrite word at the adr address */
/* adr may be any pointer type */
/* towrite should be 32-bit variable or constant (not the expression) */
/* BE_LOAD_WORD loads the toread word from the adr address */
#ifdef BE_MACHINE

#define BE_STORE_WORD(adr, towrite)				\
			*(unsigned int *)(adr)=(towrite)

#define BE_LOAD_WORD(adr, toread)				\
			(toread)=*(unsigned int *)(adr)

#else

#define BE_STORE_WORD(adr, towrite)				\
			{									\
				BYTE * const bp=(BYTE *)(adr);	\
				bp[0]=(BYTE)((towrite)>>24);	\
				bp[1]=(BYTE)((towrite)>>16);	\
				bp[2]=(BYTE)((towrite)>>8);		\
				bp[3]=(BYTE)(towrite);			\
			}

#define BE_LOAD_WORD(adr, toread)						\
			{											\
				const BYTE * const bp=(BYTE *)(adr);	\
				(toread)= (((unsigned int)bp[0])<<24)	\
						| (((unsigned int)bp[1])<<16)	\
						| (((unsigned int)bp[2])<<8)	\
						|  ((unsigned int)bp[3]);		\
			}

#endif

/* return CPU Endianness 1-BigEndian, 0-Little */
int BEmachine();

/* endian-aware*/
/* convert raw BigEndian pixels (either 1 or 2 Bytes/pixel) to actual Endian 2-Byte PIXELs */
/* rawBYTESpp - raw Bytes/pixel, width - pixels/row */
void BErawrowtopixelrow(const BYTE * const filerow, PIXEL * const currow, const int width, const int rawBYTESpp);

/* reverse BErawrowtopixelrow*/
void BEpixelrowtorawrow(BYTE * const filerow, const PIXEL * const currow, const int width, const int rawBYTESpp);

/* store in f the 16-bit number val as BigEndian*/
/* return EOF in case of error */
int BEwrite2Bytes(int val, FILE *f);

/* read 16-bit BigEndian number*/
int BEread2Bytes(FILE *f);

/* store in f the 32-bit number val as BigEndian*/
/* return EOF in case of error */
int BEwrite4Bytes(int val, FILE *f);

/* read 32-bit BigEndian number*/
int BEread4Bytes(FILE *f);


#endif
