#include "sfalicplugin.h"

#include <QTranslator>
#include <QApplication>

#include <fugio/global_signals.h>
#include <fugio/nodecontrolbase.h>

#include <fugio/sfalic/uuid.h>

#include "compressnode.h"
#include "decompressnode.h"

QList<QUuid>	NodeControlBase::PID_UUID;

const ClassEntry SFALICPlugin::mNodeClasses[] =
{
	ClassEntry( "Compress", "SFALIC", NID_SFALIC_COMPRESS, &CompressNode::staticMetaObject ),
	ClassEntry( "Decompress", "SFALIC", NID_SFALIC_DECOMPRESS, &DecompressNode::staticMetaObject ),
	ClassEntry()
};

const ClassEntry SFALICPlugin::mPinClasses[] =
{
	ClassEntry()
};

SFALICPlugin::SFALICPlugin()
{
	//-------------------------------------------------------------------------
	// Install translator

	static QTranslator		Translator;

	if( Translator.load( QLocale(), QLatin1String( "translations" ), QLatin1String( "_" ), ":/" ) )
	{
		qApp->installTranslator( &Translator );
	}
}

PluginInterface::InitResult SFALICPlugin::initialise( fugio::GlobalInterface *pApp, bool pLastChance )
{
	Q_UNUSED( pLastChance )

	mApp = pApp;

	mApp->registerNodeClasses( mNodeClasses );

	mApp->registerPinClasses( mPinClasses );

	return( INIT_OK );
}

void SFALICPlugin::deinitialise( void )
{
	mApp->unregisterPinClasses( mPinClasses );

	mApp->unregisterNodeClasses( mNodeClasses );

	mApp = Q_NULLPTR;
}
