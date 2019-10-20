#ifndef DECOMPRESSNODE_H
#define DECOMPRESSNODE_H

#include <fugio/nodecontrolbase.h>
#include <fugio/image/image.h>

class DecompressNode : public fugio::NodeControlBase
{
	Q_OBJECT
	Q_CLASSINFO( "Author", "Alex May" )
	Q_CLASSINFO( "Version", "1.0" )
	Q_CLASSINFO( "Description", "Deompresses a grayscale image using the SFALIC algorithm" )
	Q_CLASSINFO( "URL", "http://wiki.bigfug.com/Decompress_(SFALIC)" )
	Q_CLASSINFO( "Contact", "http://www.bigfug.com/contact/" )

public:
	Q_INVOKABLE DecompressNode( QSharedPointer<fugio::NodeInterface> pNode );

	virtual ~DecompressNode( void ) Q_DECL_OVERRIDE {}

	// NodeControlInterface interface

	virtual void inputsUpdated( qint64 pTimeStamp ) Q_DECL_OVERRIDE;

private:
	void decompress( qint64 pTimeStamp, const QByteArray &pDatBuf );

protected:
	QSharedPointer<fugio::PinInterface>			 mPinInputBuffer;

	QSharedPointer<fugio::PinInterface>			 mPinOutputImage;
	fugio::VariantInterface						*mValOutputImage;

	QByteArray									 mDatBuf;
};

#endif // DECOMPRESSNODE_H
