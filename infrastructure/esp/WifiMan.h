#ifndef WIFIMAN_H
#define WIFIMAN_H
//#include <Ticker.h>

#include "CurFSEsp8266.h"

#include "../SyncedClock.h"

#include "NTPSyncer.h"
#undef FTEMPLATE
#define FTEMPLATE ".irom.text.wifiman"
//#define FF(string_literal) (reinterpret_cast<const __FlashStringHelper *>(((__extension__({static const char __c[] __attribute__((section(".irom.text.wifiman"))) = ((string_literal)); &__c[0];})))))
//#define RF(x) String(FF(x)).c_str()


#define WIFITIMEOUT 12
#define PASSFIELD "wifi-password"
#define SSIDFIELD "wifi-ssid"
#define APNAMEFIELD "wifi-apname"




class WifiMan;

//void ntpSyncCB(uint32_t sec,bool connected1);

class WifiListener: public NTPListener{
public:
	WifiMan *mywifiman=0;
	WifiListener(WifiMan *mywifiman0, NTPSyncer *mysyncer0):NTPListener(mysyncer0), mywifiman(mywifiman0){}
	void ntpUpdate(uint32_t sec,bool connected1);
};

class WifiMan
{
	NTPSyncer syncer;
	WifiListener wifilistener;
	Ticker wifiTicker;
	//	std::map<std::string,std::string > wificonfig; // waste of memory
	//	std::string ssid,passwd,apname;

	uint32_t m0,mp;
	//	int32_t mdiff;

	uint32_t syncsec=0;
	int mintosync;

	uint32_t timeout=WIFITIMEOUT,pingretryto=10;//sec
	bool reconnecting=false;
	bool pingretry=false;
	int retries=0;
	GenMap *config;

public:
	bool  connectedInternet=false, ntpresponse=false;

	WifiMan(void);
	bool initFromConfig(GenMap *config);
	//bool initFromFile(std::string passfile);
	//bool init(std::string ssid,std::string pass);
	bool init();
	void reconnect();
	void startAP();
	void stopAP();
	static void reconnectIfRequiredStat(WifiMan *);
	void reconnectIfRequired();
	void saveConfig(std::map<std::string,std::string >&config);
};


void WifiListener::ntpUpdate(uint32_t sec,bool connected1){

	mywifiman->connectedInternet=connected1;
	if(connected1 && sec){
		int32_t mdiff=mysyncer->getSec()-CLOCK32.getMS()/1000;
		if(abs(mdiff)>2) CLOCK32.resyncSec(sec);
		else println(std::string()+
				RF("NTP TS dismissed, diff :")+
				to_string(mdiff));
	}
	mywifiman->ntpresponse=true;
};


WifiMan::WifiMan(): wifilistener(this,&syncer) {
	//wifilistener=new WifiListener(this,&syncer);
}
/*
~WifiMan::WifiMan(){
//	if(wifilistener) {delete wifilistener;wifilistener=0;}
}
 */
void WifiMan::reconnectIfRequiredStat(WifiMan *wman){wman->reconnectIfRequired();}
void WifiMan::reconnectIfRequired(){
	//print("wifi status : ");println(WiFi.status());
	if(WiFi.status() == WL_DISCONNECTED && (millis()-m0)>timeout*1000){
		GenString passwd=(*config)[RF(PASSFIELD)];
		GenString ssid=(*config)[RF(SSIDFIELD)];

		// restart AP
		if(!reconnecting) startAP();
		// try to connect
		println(RF("attempt to reconnect to wifi"));
		WiFi.disconnect(true);
		WiFi.begin(ssid.c_str(), passwd.c_str());
		m0=millis();
		reconnecting=true;
	}

	if(reconnecting && WiFi.status() == WL_CONNECTED){
		stopAP();reconnecting=false;
		println(RF("pinger started"));
		syncer.pingSyncNTP(&wifilistener);
		ntpresponse=false;
		retries=10;
	}
	if(ntpresponse && !connectedInternet ){	// failed retry later
		mp=millis();
		ntpresponse=false;
		pingretry=true;
		retries--;
	}
	if(retries>0 && pingretry && (millis()-mp)>pingretryto*1000) {	// retry now
		println(RF("pinger restarted"));
		syncer.pingSyncNTP(&wifilistener);
		ntpresponse=false;
		pingretry=false;
	}

}

void WifiMan::reconnect(){
	if(WiFi.status() != WL_CONNECTED){
		// try to connect
		GenString passwd=(*config)[RF(PASSFIELD)];
		GenString ssid=(*config)[RF(SSIDFIELD)];
		WiFi.begin(ssid.c_str(), passwd.c_str());
		m0=millis();
		wifiTicker.detach();
		wifiTicker.attach_ms(1000,reconnectIfRequiredStat,this);
	}
}
void  WifiMan::stopAP(){
	WiFi.mode(WIFI_STA);
	println(RF("AP stopped"));
}

void  WifiMan::startAP(){
	GenString apname=(*config)[RF(APNAMEFIELD)];
	if(apname.empty()) apname=RF("ioktopusAP");	//move to constructor ?
	WiFi.mode(WIFI_AP_STA);
	WiFi.softAP(apname.c_str());
}

//std::map<std::string,std::string > parsePassFile(unsigned char *buff){return {{RF("ssid"),RF("Gardening, cheaper than therapy")},{RF("password"),RF("seeds freedom")},{RF("apname"),RF("IOktopus-AP")}};}

bool WifiMan::initFromConfig(GenMap *config0){
	println(RF("Connecting wifi / setup access point..."));
	config=config0;
	init();
}
/*
bool WifiMan::initFromFile(std::string passfile)
{

	println(RF("Connecting wifi / setup access point..."));

	// TODO: start station & AP, switch off AP when connected to station, switch back on when disconnected from station,
	// what happens to the server is wifi is not connected/softAP not connected ?
	//	WiFi.disconnect(true);

	// load ssid/pass file
	size_t sz;

	unsigned char *buff=CURFS.readFileBuffer(passfile,sz);  // sram

 	std::map<std::string,std::string >config=parsePassFile(buff);//JSONMAP(std::string((const char *)buff,sz));
	delete buff;

	 saveConfig(config);

 	init();
	// sram WiFi library use
	/*WiFi.begin(ssid.c_str(), passwd.c_str());
	WiFi.status();
	WiFi.localIP();
	WiFi.mode(WIFI_AP_STA);
	WiFi.softAP(apname.c_str());* /
}
void WifiMan::saveConfig(std::map<std::string,std::string >&config){
	passwd=config[RF(PASSFIELD)];
	ssid=config[RF(SSIDFIELD)];
	apname=config[RF(APNAMEFIELD)];
}
bool WifiMan::init(std::string ssid, std::string password)
{
	std::map<std::string,std::string >config={{RF("ssid"),RF("Gardening, cheaper than therapy")},{RF("password"),RF("seeds freedom")},{RF("espap"),RF("IOktopus-AP")}};
	saveConfig(config);
	init();

}*/

bool WifiMan::init()
{
	GenString passwd=(*config)[RF(PASSFIELD)];
	GenString ssid=(*config)[RF(SSIDFIELD)];
	//	GenString apname=config[RF(APNAMEFIELD)];

	if(ssid.empty()) ssid=RF("noconfigfile");
	if(passwd.empty()) passwd="";

	startAP();

	bool synced=false;
	println(WiFi.status());


	WiFi.begin(ssid.c_str(), passwd.c_str());



	println(std::string()+RF("Connecting to wifi ssid : '")+ssid.c_str()+RF("'"));
	m0=millis();
	print(RF("Connecting "));

	while ((millis()-m0)<timeout*1000 && WiFi.status() == WL_DISCONNECTED)
	{
		delay(500);
		print(RF("."));
		//	if(WiFi.status()==1) println(std::string()+"Ready "+to_string(WiFi.status()));
		//		if(WiFi.status()==6) println(std::string()+"Connecting "+to_string(WiFi.status()));
	}
	bool retry=true;
	if(WiFi.status() == WL_CONNECTED){
		println(std::string()+RF("connected to wifi in ")+to_string(millis()-m0)+RF("ms"));
		print(RF("IP Address is: "));
		println(WiFi.localIP());
		stopAP();
			println("pinger started");

		syncer.pingSyncNTP(&wifilistener);

		ntpresponse=false;
		while(!ntpresponse) delay(100);
		ntpresponse=false;
		synced=true;

	} else{
		println(GenString() + RF("Failed to connect to wifi ssid :")+ssid+" with password '"+passwd+"'");
		if(WiFi.status() ==WL_IDLE_STATUS) println(RF("ssid not detected"));	//could stop STA here and restart every while (how long?) to see if ssid is back
		if(WiFi.status() ==WL_CONNECT_FAILED) {println(RF("password rejected"));retry=false;}//could stop STA here until next change of password or next retry after timeout
	}

	if(retry) wifiTicker.attach_ms(1000,reconnectIfRequiredStat, this);

	//mdiff=syncer.getSec()-CLOCK.getTimeMS()/1000;
	return synced;
}


/*	WiFi.mode(WIFI_AP_STA);
	WiFi.begin(ssid, password);
	WiFi.softAP(espap);
	while (WiFi.status() == WL_DISCONNECTED)
		{
			delay(500);
			print(".");
			println(WiFi.status());
		}
	return;*/


#endif
