#ifndef COMPRESSNODE_H
#define COMPRESSNODE_H

#include <fugio/nodecontrolbase.h>

#include <fugio/core/variant_interface.h>

class CompressNode : public fugio::NodeControlBase
{
	Q_OBJECT
	Q_CLASSINFO( "Author", "Alex May" )
	Q_CLASSINFO( "Version", "1.0" )
	Q_CLASSINFO( "Description", "Compresses a grayscale image using the SFALIC algorithm" )
	Q_CLASSINFO( "URL", "http://wiki.bigfug.com/Compress_(SFALIC)" )
	Q_CLASSINFO( "Contact", "http://www.bigfug.com/contact/" )

public:
	Q_INVOKABLE CompressNode( QSharedPointer<fugio::NodeInterface> pNode );

	virtual ~CompressNode( void ) {}

	// NodeControlInterface interface

	virtual void inputsUpdated( qint64 pTimeStamp ) Q_DECL_OVERRIDE;

private:
	void compress8( void );
	void compress( qint64 pTimeStamp );

protected:
	QSharedPointer<fugio::PinInterface>			 mPinInputImage;

	QSharedPointer<fugio::PinInterface>			 mPinInputBits;

	QSharedPointer<fugio::PinInterface>			 mPinInputHeader;

	QSharedPointer<fugio::PinInterface>			 mPinOutputBuffer;
	fugio::VariantInterface						*mValOutputBuffer;
};


#endif // COMPRESSNODE_H
