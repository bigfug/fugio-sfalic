#ifndef SFALIC_HEADER_H
#define SFALIC_HEADER_H

#include <fugio/global.h>

FUGIO_NAMESPACE_BEGIN

#pragma pack(push, 1)

typedef struct SFALICHeader
{
	quint32		mHeaderTag;
	quint32		mCompressedSize;
	quint16		mImageWidth, mImageHeight;
	qint8		mImageBits;
	qint8		mMaxClen, mEvol;
	qint8		mPadding;
} SFALICHeader;

#pragma pack(pop)

FUGIO_NAMESPACE_END

#endif // SFALIC_HEADER_H
