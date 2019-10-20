#ifndef __CDFTYPES_H
#define __CDFTYPES_H



typedef	signed char	i8;
typedef	unsigned char	u8;
typedef	unsigned short int	u16;

typedef u8	BYTE;


/* pixel as stored in RAM */
typedef u16	PIXEL;
/* maximum number of codes in family */
#define MAXNUMCODES 16
/* maximum allowed image depth - is 16 - do not change */
#define IMG_MAX_bpp 16


#endif /* __CDFTYPES_H */
