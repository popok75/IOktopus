#ifndef COMPATNET_H
#define COMPATNET_H


#ifdef x86BUILD
#define CurWebServer CurWebServerx86
#include "x86/CurWebServerx86.h"
#include "SyncedClock.h"

class WifiMan{public:
	static void initFromConfig(GenMap *config){
		CLOCK32.resyncSec(millis64()/1000);
	};
	static void reconnect(){};
	static void startAP(){};
	static void stopAP(){};
	static void reconnectIfRequired(){};
};
#endif


#ifdef ESP8266BUILD
#define CurWebServer CurWebServerEsp8266
#include "esp/WifiMan.h"
#include "esp/CurWebServerEsp8266.h"
#endif


#endif
