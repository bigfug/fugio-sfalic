#include "compressnode.h"

#include <fugio/image/uuid.h>
#include <fugio/core/uuid.h>
#include <fugio/image/image.h>
#include <fugio/performance.h>

extern "C"
{
#include <SFALIC/cdftypes.h>
#include <SFALIC/clalloc.h>
#include <SFALIC/cdfpred.h>
#include <SFALIC/cdfstat.h>
}

#include <QtEndian>

#include "sfalicplugin.h"

#include <fugio/sfalic/sfalic.h>

CompressNode::CompressNode( QSharedPointer<fugio::NodeInterface> pNode )
	: NodeControlBase( pNode )
{
	FUGID( PIN_INPUT_IMAGE,		"9e154e12-bcd8-4ead-95b1-5a59833bcf4e" )
	FUGID( PIN_INPUT_BITS,		"30C549C0-317E-4E8D-A375-B922E05FB99A" )
	FUGID( PIN_INPUT_HEADER,	"5CC2F099-66F3-4F26-B95A-2FC27A63A977" )
	FUGID( PIN_OUTPUT_BUFFER,	"1b5e9ce8-acb9-478d-b84b-9288ab3c42f5" )

	mPinInputImage = pinInput( "Image", PIN_INPUT_IMAGE );

	mPinInputImage->registerPinInputType( PID_IMAGE );

	mPinInputImage = pinInput( "Bits", PIN_INPUT_BITS );

	mPinInputImage = pinInput( "Header", PIN_INPUT_HEADER );

	mValOutputBuffer = pinOutput<fugio::VariantInterface *>( "Buffer", mPinOutputBuffer, PID_BYTEARRAY, PIN_OUTPUT_BUFFER );
}

void CompressNode::inputsUpdated( qint64 pTimeStamp )
{
	if( mPinInputImage->isUpdated( pTimeStamp ) )
	{
		fugio::Image		ImgInt = variant<fugio::Image>( mPinInputImage );

		if( !ImgInt.isValid() )
		{
			return;
		}

		if( ImgInt.format() != fugio::ImageFormat::GRAY8 &&
				ImgInt.format() != fugio::ImageFormat::GRAY16 )
		{
			return;
		}

		if( ImgInt.format() == fugio::ImageFormat::GRAY8 )
		{

		}
		else
		{
			compress( pTimeStamp );
		}
	}
}

void CompressNode::compress8()
{

}

void CompressNode::compress( qint64 pTimeStamp )
{
	fugio::Performance	Perf( mNode, "compress", pTimeStamp );

	fugio::Image		ImgInt = variant<fugio::Image>( mPinInputImage );

	const int			width = ImgInt.width();
	const int			height = ImgInt.height();
	const int			fileBYTESpp = ( ImgInt.format() == fugio::ImageFormat::GRAY16 ? 2 : 1 );

	int					ImgBits = variant( mPinInputBits ).toInt();

	if( ImgBits <= 0 )
	{
		ImgBits = fileBYTESpp * 8;
	}

	if( ImgBits > 16 )
	{
		return;
	}

	if( fileBYTESpp == 1 && ImgBits > 8 )
	{
		ImgBits = 8;
	}

	const bool			AddHdr = variant( mPinInputHeader ).toBool();

	QVector<PIXEL>		prevrow( width ); /* previous row */
	QVector<PIXEL>		currow( width );  /* current row */
	QVector<PIXEL>		decorelatedrow( width );	/* decorrelated row */
	QVector<quint32>	compressedrow( width + 1 );	/* compressed row - max len=4B/pixel+4B */

	PIXEL				context;  /* context of the 1st pixel in the row */
	int					row;		/* current row nr */
	unsigned int		bitsused;	/* for bit-io number of bits output and not written yet to the buffer */
	int					needconvert;		/* is conversion from raw image data to pixel type required? */

	QByteArray			OutBuf;

	SFALICHeader		OutHdr;

	OutHdr.mHeaderTag   = 123;
	OutHdr.mImageWidth  = width;
	OutHdr.mImageHeight = height;
	OutHdr.mImageBits   = ImgBits;
	OutHdr.mMaxClen     = DEFmaxclen;
	OutHdr.mEvol        = DEFevol;
	OutHdr.mPadding     = 0;

	if( AddHdr )
	{
		OutBuf.append( reinterpret_cast<const char *>( &OutHdr ), sizeof( OutHdr ) );
	}

	context=0;
	bitsused=0;

	needconvert=(fileBYTESpp!=sizeof(PIXEL)) || (fileBYTESpp>1 && !BEmachine());

	SFALICData		*EncDat = statinitcoder( ImgBits, DEFmaxclen, DEFevol, 0 );

	if( !EncDat )
	{
		return;
	}

	for( row = 0 ; row < height ; row++ )
	{
		prevrow.swap( currow );

		BErawrowtopixelrow( ImgInt.buffer( 0 ) + ( row * ImgInt.lineSize( 0 ) ), currow.data(), width, 2 );

		decorelaterow( prevrow.constData(), currow.constData(), row, width, 16, DEFpred, decorelatedrow.data() );

		unsigned int fullbytes = 0;

		statcompressrow( EncDat, context, decorelatedrow.constData(), width, reinterpret_cast<quint8 *>( compressedrow.data() ), &fullbytes, &bitsused );

		OutBuf.append( reinterpret_cast<const char *>( compressedrow.constData() ), fullbytes );

		if( bitsused ) /* bitsused is in range 0-31 */
		{
			compressedrow[ 0 ] = compressedrow[ fullbytes / sizeof( quint32 ) ];
		}

		context = decorelatedrow[ 0 ];
	}

	if( bitsused ) /* bitsused is in range 0-31 */
	{
		OutBuf.append( reinterpret_cast<const char *>( compressedrow.constData() ), ( bitsused + 7 ) / 8 );
	}

	statfreecoder( EncDat );

	if( AddHdr )
	{
		OutHdr.mCompressedSize = OutBuf.size() - sizeof( OutHdr );

		memcpy( OutBuf.data(), &OutHdr, sizeof( OutHdr ) );
	}

	mValOutputBuffer->setVariant( OutBuf );

	pinUpdated( mPinOutputBuffer );
}
