#ifndef COMPATNET_H
#define COMPATNET_H


#ifdef x86BUILD

#define CurWebServer CurWebServerx86
#include "x86/CurWebServerx86.h"

#define CurWebClient CurWebClientx86
#include "x86/CurWebClientx86.h"

#include "SyncedClock.h"

class WifiMan : public EventListener {
public:
	bool notify(std::string, Event*){return false;};
	static void initFromConfig(GenMap *config){
		CLOCK32.resyncSec(millis64()/1000);	//cause up to 1second lag with real time, but simulate NTP resync on esp
		//CLOCK32.resyncMS(millis64());

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

#define CurWebClient CurWebClientEsp8266
#include "esp/CurWebClientEsp8266.h"

#endif


#endif
