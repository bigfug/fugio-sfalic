#ifndef SFALICPLUGIN_H
#define SFALICPLUGIN_H

// SFALIC: http://sun.aei.polsl.pl/~rstaros/sfalic/

#include <fugio/global_interface.h>
#include <fugio/plugin_interface.h>

#include <config.h>

using namespace fugio;

class SFALICPlugin : public QObject, public PluginInterface
{
	Q_OBJECT
	Q_INTERFACES( fugio::PluginInterface )

	Q_PLUGIN_METADATA( IID "com.bigfug.fugio.sfalic.plugin" FILE "manifest.json" )

public:
	Q_INVOKABLE explicit SFALICPlugin( void );

	virtual ~SFALICPlugin( void ) {}

	//-------------------------------------------------------------------------
	// fugio::PluginInterface

	virtual InitResult initialise( fugio::GlobalInterface *pApp, bool pLastChance );

	virtual void deinitialise( void );

private:
	GlobalInterface					*mApp;

	static const fugio::ClassEntry	 mNodeClasses[];
	static const fugio::ClassEntry	 mPinClasses[];
};

#endif // SFALICPLUGIN_H
