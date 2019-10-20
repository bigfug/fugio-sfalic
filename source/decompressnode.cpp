#include "decompressnode.h"

#include <fugio/core/uuid.h>
#include <fugio/core/variant_interface.h>
#include <fugio/image/uuid.h>

#include <fugio/performance.h>

extern "C"
{
#include <SFALIC/cdftypes.h>
#include <SFALIC/clalloc.h>
#include <SFALIC/cdfpred.h>
#include <SFALIC/cdfstat.h>
}

#include <fugio/sfalic/sfalic.h>

/* minimum size of bit-input buffer for decompression() */
#define INBUFMIN 16384

DecompressNode::DecompressNode( QSharedPointer<fugio::NodeInterface> pNode )
	: NodeControlBase( pNode )
{
	FUGID( PIN_INPUT_BUFFER,	"1b5e9ce8-acb9-478d-b84b-9288ab3c42f5" );
	FUGID( PIN_OUTPUT_IMAGE,	"9e154e12-bcd8-4ead-95b1-5a59833bcf4e" );

	mPinInputBuffer = pinInput( "Buffer", PIN_INPUT_BUFFER );

	mValOutputImage = pinOutput<fugio::VariantInterface *>( "Image", mPinOutputImage, PID_IMAGE, PIN_OUTPUT_IMAGE );
}

void DecompressNode::inputsUpdated( qint64 pTimeStamp )
{
	if( mPinInputBuffer->isUpdated( pTimeStamp ) )
	{
		fugio::VariantInterface		*VarInt = input<fugio::VariantInterface *>( mPinInputBuffer );

		if( !VarInt )
		{
			return;
		}

		const QVariant				 VarVal = VarInt->variant();

		if( VarVal.type() == QVariant::List )
		{
			const QVariantList		VarLst = VarVal.toList();

			if( !VarLst.isEmpty() )
			{
				const QVariant				 VarFin = VarLst.last();

				if( VarFin.type() == QVariant::ByteArray )
				{
					decompress( pTimeStamp, VarFin.toByteArray() );
				}
			}
		}
		else if( VarVal.type() == QVariant::ByteArray )
		{
			decompress( pTimeStamp, VarVal.toByteArray() );
		}
		else
		{
			return;
		}
	}
}

void DecompressNode::decompress( qint64 pTimeStamp, const QByteArray &pDatBuf )
{
	fugio::Performance		Perf( mNode, "decompress", pTimeStamp );

	int		width  = 640;
	int		height = 480;
	int		bpp    = 16;

	fugio::Image	DstImg = mValOutputImage->variant().value<fugio::Image>();

	DstImg.setSize( width, height );

	switch( bpp )
	{
		case 8:
			DstImg.setFormat( fugio::ImageFormat::GRAY8 );
			DstImg.setLineSize( 0, width );
			break;

		case 16:
			DstImg.setFormat( fugio::ImageFormat::GRAY16 );
			DstImg.setLineSize( 0, width * 2 );
			break;

		default:
			return;
	}

	quint8			*DstBuf = DstImg.internalBuffer( 0 );

	if( !DstBuf )
	{
		return;
	}

	const int		 DstMax = DstImg.bufferSize( 0 );
	int				 DstCnt = 0;

	const quint8	*SrcBuf = reinterpret_cast<const quint8 *>( pDatBuf.constData() );
	const int		 SrcMax = pDatBuf.size();
	int				 SrcCnt = 0;

	qDebug() << "received" << pDatBuf.size() << "bytes";

	int fileBYTESpp;
	int compressedrowlen=4*(width+2);
	PIXEL context;
	int row;
	struct bitinstatus bs;
	int needconvert;
	int eof=0;

	if (compressedrowlen<INBUFMIN)
		compressedrowlen=INBUFMIN;

	//	assert(!decoronly);

	if (bpp<9)
		fileBYTESpp=1;
	else
		fileBYTESpp=2;

	needconvert=(fileBYTESpp!=sizeof(PIXEL)) || (fileBYTESpp>1 && !BEmachine());

	context=0;

	QVector<quint8>	filerow( width * fileBYTESpp );
	QVector<quint8>	compressedrow( compressedrowlen );
	QVector<PIXEL>	prevrow( width );
	QVector<PIXEL>	currow( width );
	QVector<PIXEL>	decorelatedrow( width );

	//eof=(compressedrowlen!=fread((void *)compressedrow, 1, compressedrowlen, infile));

	if( true )
	{
		const int	 ReadAmount = qMin( compressedrowlen, SrcMax - SrcCnt );

		memcpy( compressedrow.data(), SrcBuf + SrcCnt, ReadAmount );

		SrcCnt += ReadAmount;

		eof = compressedrowlen != ReadAmount;
	}

	bs.rdptr = compressedrow.data();

	SFALICData	*DecDat = statinitdecoder( bpp, DEFmaxclen, DEFevol, width, &bs );

	if( !DecDat )
	{
		return;
	}

	for( row = 0 ; row < height ; row++ )
	{
		int			usedbytes;

		if( statdecompressrow( DecDat, context, decorelatedrow.data(), width, &bs ) )
		{
			// exitit("error decompressing (statdecompressrow())", 4);

			statfreedecoder( DecDat );

			return;
		}

		usedbytes = int( bs.rdptr - compressedrow.constData() );

		if( usedbytes > compressedrowlen )
		{
			// exitit("error decompressing image", 4);

			statfreedecoder( DecDat );

			return;
		}

		if( ( compressedrowlen - usedbytes < 4 * width ) && ( !eof ) ) /* input buffer may contain less bits than compressed image row - fill needed */
		{
			memmove( compressedrow.data(), compressedrow.constData() + usedbytes, compressedrowlen - usedbytes );

			bs.rdptr = compressedrow.data();

			void		*ReadDest = (void *)( compressedrow.data() + ( compressedrowlen - usedbytes ) );
			const int	 ReadAmount = qMin( usedbytes, SrcMax - SrcCnt );

			memcpy( ReadDest, SrcBuf + SrcCnt, ReadAmount );

			SrcCnt += ReadAmount;

			eof = usedbytes != ReadAmount;
		}

		context = decorelatedrow[ 0 ];

		corelaterow( prevrow.constData(), currow.data(), row, width, bpp, DEFpred, decorelatedrow.constData() );

		const void		*SrcPtr = nullptr;

		if( needconvert )
		{
			BEpixelrowtorawrow( filerow.data(), currow.constData(), width, fileBYTESpp );

			SrcPtr = filerow.constData();
		}
		else
		{
			SrcPtr = currow.constData();
		}

		if( DstCnt + ( fileBYTESpp * width ) > DstMax )
		{
			mNode->setStatusMessage( tr( "error writing pixels to outfile" ) );

			statfreedecoder( DecDat );

			return;
		}

		memcpy( DstBuf + DstCnt, SrcPtr, fileBYTESpp * width );

		DstCnt += ( fileBYTESpp * width );

		prevrow.swap( currow );
	}

	statfreedecoder( DecDat );

	pinUpdated( mPinOutputImage );
}
