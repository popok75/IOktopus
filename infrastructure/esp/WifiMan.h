#ifndef WIFIMAN_H
#define WIFIMAN_H
//#include <Ticker.h>

#define WIFIMANUSEMDNS false

#include "CurFSEsp8266.h"
#include <ESP8266mDNS.h>
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
#define APPASSFIELD "wifi-appassword"

#include "../events/EventEmitter.h"

#define GETSTATIONSTATUS "getStationStatus"
#define GETSTATIONIP "getStationIP"
#define GETSOFTAPIP "getSoftAPIP"
#define CONNECTSTATION "ConnectStation"
#define DISCONNECTSTATION "DisconnectStation"
#define CONNECTAP "ConnectAP"
#define DISCONNECTAP "DisconnectAP"

#define STATION_STATUS_DISCONNECTED 0
#define STATION_STATUS_CONNECTING 1
#define STATION_STATUS_CONNECTED 2
#define STATION_STATUS_CONNECTED_INTERNET 3
#define STATION_STATUS_PASSWORD_REJECTED 4
#define STATION_STATUS_SSID_UNAVAILABLE 5

#define STATION_STATUS_DISCONNECTED_TEXT "STATION_STATUS_DISCONNECTED"
#define STATION_STATUS_CONNECTING_TEXT "STATION_STATUS_CONNECTING"
#define STATION_STATUS_CONNECTED_TEXT "STATION_STATUS_CONNECTED"
#define STATION_STATUS_CONNECTED_INTERNET_TEXT "STATION_STATUS_CONNECTED_INTERNET"
#define STATION_STATUS_PASSWORD_REJECTED_TEXT "STATION_STATUS_PASSWORD_REJECTED"
#define STATION_STATUS_SSID_UNAVAILABLE_TEXT "STATION_STATUS_SSID_UNAVAILABLE"

class WifiMan;


/*
 WifiMan module will try to create an AP when station connection cannot be established
 	 - on start, will start an AP, then attempt a station connection, if established ap is stopped, or else ap is kept
 	 - if station is established AP is stopped by default. It can later be actived alone.
 	 - if station is stopped and no ap is running, start an ap
 	 - if ap is alone and stopped, the board will become unreachable


 * */


//void ntpSyncCB(uint32_t sec,bool connected1);

class WifiListener: public NTPListener{
public:
	WifiMan *mywifiman=0;
	WifiListener(WifiMan *mywifiman0, NTPSyncer *mysyncer0):NTPListener(mysyncer0), mywifiman(mywifiman0){}
	void ntpUpdate(uint32_t sec,bool connected1);
	void pingUpdate(bool online);
};

class WifiMan : public EventListener {
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
	GenString state="none"; // state of the wifi module
	unsigned stationStatus=STATION_STATUS_DISCONNECTED;	// state of the station connection

	GenString wifimanpasswd, wifimanssid;

public:
	bool connectedstation=false, wrongpassword=false, ssidunavail=false, ntpsynced=false;
	bool connectedInternet=false, ntpresponse=false;
	bool pingsuccess=false, pingupdated=false;
	bool autostopAP=true;

	WifiMan(void);
	bool initFromConfig(GenMap *config);
	//bool initFromFile(std::string passfile);
	//bool init(std::string ssid,std::string pass);
	bool init();
	void reconnect();
	void yield();
	void startAP();
	void stopAP();
	bool connectStationAndWait();
	bool connectStation();
//	bool connectStation2();
	void disconnectStation();
	static void reconnectIfRequiredStat(WifiMan *);
	void reconnectIfRequired();
	void saveConfig(std::map<std::string,std::string >&config);
	bool notify(GenString ename,Event*event=0);//{return false;};
};



void WifiMan::yield(){
//	Serial.println(String()+"Wifi Status:"+WiFi.status());
	bool test=false;
	switch(WiFi.status()) {
		case WL_CONNECT_FAILED:
			wrongpassword=true;test=true;
			stationStatus=STATION_STATUS_PASSWORD_REJECTED;
		case WL_NO_SSID_AVAIL:
			if(!test) {ssidunavail=true;stationStatus=STATION_STATUS_SSID_UNAVAILABLE;}
			println(std::string()+RF("Failed to connect to wifi ")+wifimanssid+RF(" with password ")+wifimanpasswd);
			Serial.println(String()+"Wifi error Status:"+WiFi.status());
			WiFi.disconnect();
			break;
		case WL_CONNECTED:
			if(!connectedstation){
				stationStatus=STATION_STATUS_CONNECTED;
				connectedstation=true;
				println(std::string()+RF("connected to wifi ")+wifimanssid+RF(" in ")+to_string(millis()-m0)+RF("ms"));
				print(RF("IP Address is: "));
				println(WiFi.localIP());
				if(autostopAP) stopAP();
				println("pinger starting");

				syncer.pingSyncNTP(&wifilistener);
				println("sent NTP request");
				ntpresponse=false;

			/*	while(!ntpresponse) delay(100);
				ntpresponse=false;
				synced=true;*/
			}
			break;
		default:break;
	}
	if(ntpresponse) {ntpresponse=false;ntpsynced=true;stationStatus=STATION_STATUS_CONNECTED_INTERNET;}

	reconnectIfRequired();
//	if(retry) wifiTicker.attach_ms(1000,reconnectIfRequiredStat, this);
//	println(GenString()+"WifiState:"+state);
	//mdiff=syncer.getSec()-CLOCK.getTimeMS()/1000;
}

bool WifiMan::notify(GenString ename,Event*event){
	println(std::string()+RF("Wifiman notify : ")+ename);
	struct anon {
		bool fillEvent(Event*event, GenString val){
			StringEvent *strev=0;
			if(event->getClassType()==StringEventTYPE) strev = (StringEvent*)(event);
			if(!strev) return false;
			strev->str=val;
			return true;
		};
	} func;

	if(ename==RF(GETSTATIONIP) || ename==RF(GETSOFTAPIP)){
/*		StringEvent *strev=0;
		if(event->getClassType()==StringEventTYPE) strev = (StringEvent*)(event);
		if(!strev) return false;
		if(ename==RF(GETSTATIONIP)) strev->str=WiFi.localIP().toString().c_str();
		else strev->str=WiFi.softAPIP().toString().c_str();
	*/
		bool b=false;
		if(ename==RF(GETSTATIONIP)) b=func.fillEvent(event,WiFi.localIP().toString().c_str());
		else b=func.fillEvent(event,WiFi.softAPIP().toString().c_str());
		StringEvent *strev=0;
		if(event->getClassType()==StringEventTYPE) {
			strev = (StringEvent*)(event);
			println(std::string()+RF("Wifiman notify : ")+ename+" "+strev->str);
		}

		return b;
	}
	if(ename==RF(GETSTATIONSTATUS)){
		 switch(stationStatus){
			 case STATION_STATUS_DISCONNECTED:func.fillEvent(event,RF(STATION_STATUS_DISCONNECTED_TEXT));break;
			 case STATION_STATUS_CONNECTING:func.fillEvent(event,RF(STATION_STATUS_CONNECTING_TEXT));break;
			 case STATION_STATUS_CONNECTED:func.fillEvent(event,RF(STATION_STATUS_CONNECTED_TEXT));break;
			 case STATION_STATUS_CONNECTED_INTERNET:func.fillEvent(event,RF(STATION_STATUS_CONNECTED_INTERNET_TEXT));break;
			 case STATION_STATUS_PASSWORD_REJECTED:func.fillEvent(event,RF(STATION_STATUS_PASSWORD_REJECTED_TEXT));break;
			 case STATION_STATUS_SSID_UNAVAILABLE:func.fillEvent(event,RF(STATION_STATUS_SSID_UNAVAILABLE_TEXT));break;
			  default:break;
		 }
		 return true;
	}
	if(ename==RF(CONNECTSTATION)){
		bool b=connectStation();
		if(b) func.fillEvent(event,"OK");
		else func.fillEvent(event,"NOTOK");
	}
	if(ename==RF(DISCONNECTSTATION)){
		disconnectStation();
		func.fillEvent(event,"OK");
	}
	if(ename==RF(CONNECTAP)){
		startAP();
		func.fillEvent(event,"OK");
	}
	if(ename==RF(DISCONNECTAP)){
		stopAP();
		func.fillEvent(event,"OK");
	}

	return false;
};

void WifiListener::pingUpdate(bool connected){
	mywifiman->pingupdated=true;
	mywifiman->pingsuccess=connected;

}
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
	if(WiFi.status() == WL_DISCONNECTED && (state==RF("STA") || state==RF("AP_STA")) && (millis()-m0)>timeout*1000){
		GenString passwd=(*config)[RF(PASSFIELD)];
		GenString ssid=(*config)[RF(SSIDFIELD)];

		// restart AP
		if(!reconnecting) startAP();
		// try to connect
		println(std::string()+RF("attempt to reconnect to wifi ")+ssid);
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
	//WiFi.mode(WIFI_STA);
	if(state==RF("AP_STA")){WiFi.mode(WIFI_STA);state=RF("STA");}
	else if(state==RF("AP")) {WiFi.mode(WIFI_OFF);state=RF("none");}	//this is probably undesirable
	println(RF("WifiMan::stopAP: AP stopped"));
	println(GenString()+"WifiState:"+state);
}

void  WifiMan::startAP(){
	GenString apname=(*config)[RF(APNAMEFIELD)];
	GenString appass=(*config)[RF(APPASSFIELD)];

	println(GenString()+RF("WifiMan::startAP: AP started ")+apname+"/"+appass);

	if(apname.empty()) apname=RF("ioktopusAP");	//move to constructor ?
	if(state==RF("STA") || state==RF("AP_STA")) {
		WiFi.mode(WIFI_AP_STA);state=RF("AP_STA");
	}
	else {WiFi.mode(WIFI_AP);state==RF("AP");}
	WiFi.softAP(apname.c_str(),appass.c_str());
	print(RF("WifiMan::startAP:AP started with ip:"));
	println(WiFi.softAPIP());
	println(GenString()+"WifiState:"+state);
}

//std::map<std::string,std::string > parsePassFile(unsigned char *buff){return {{RF("ssid"),RF("Gardening, cheaper than therapy")},{RF("password"),RF("seeds freedom")},{RF("apname"),RF("IOktopus-AP")}};}

bool WifiMan::initFromConfig(GenMap *config0){
	println(RF("Connecting wifi / setup access point..."));
	config=config0;
	return init();
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
	/ *WiFi.begin(ssid.c_str(), passwd.c_str());
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
{	//	WiFi.persistent(true);	//mDNS require this ??  : https://github.com/esp8266/Arduino/issues/1950
	startAP();

	println(WiFi.status());
	return connectStationAndWait();
}

void WifiMan::disconnectStation()
{
	if(state==RF("AP_STA")){WiFi.mode(WIFI_AP);state=RF("AP");}
	else if(state==RF("STA")) {WiFi.mode(WIFI_OFF);state=RF("none");}	//this is probably undesirable
	println(RF("Station disconnected"));
	println(GenString()+"WifiState:"+state);
	if(state==RF("none")) startAP();	// if no ap running start one
	println(GenString()+"WifiState2:"+state);
}

bool WifiMan::connectStationAndWait()
{
	connectStation();
	while ((millis()-m0)<timeout*1000 && WiFi.status() == WL_DISCONNECTED)
	{
		yield();
		delay(500);
		print(RF("."));
	}
	println();
	println(RF("Waiting for ntp response"));
	while(WiFi.status() == WL_CONNECTED && !ntpsynced) {yield();delay(500);print(RF("."));}
	println();
	return WiFi.status() == WL_CONNECTED;
}


bool WifiMan::connectStation()
{

	if(state==RF("none")){/*WiFi.mode(WIFI_STA);*/state=RF("STA");}
	if(state==RF("AP")){/*WiFi.mode(WIFI_AP_STA);*/state=RF("AP_STA");}

	wifimanpasswd=(*config)[RF(PASSFIELD)];
	wifimanssid=(*config)[RF(SSIDFIELD)];
	//	GenString apname=config[RF(APNAMEFIELD)];

	if(wifimanssid.empty()) wifimanssid=RF("noconfigfile");
	if(wifimanpasswd.empty()) wifimanpasswd="";
	connectedstation=false;ssidunavail=false;wrongpassword=false;
	WiFi.begin(wifimanssid.c_str(), wifimanpasswd.c_str());
	stationStatus=STATION_STATUS_CONNECTING;

	println(std::string()+RF("Connecting to wifi ssid : '")+wifimanssid.c_str()+RF("'"));
	m0=millis();
	return false;
}
/*
bool connectST(const char *ssid1,const char *password1){
	// Connect to WiFi network
	// if(apon) WiFi.mode(WIFI_AP_STA);
	// else WiFi.mode(WIFI_STA);
	// staon=true;
	WiFi.mode(WIFI_AP_STA);

	WiFi.begin(ssid1, password1);
	Serial.println("");
	unsigned int ts=millis(), timeout=30000;
	// Wait for connection
	while (WiFi.status() != WL_CONNECTED && (millis()-ts)<timeout) {
		delay(500);
		Serial.print(".");
	}
	if( WiFi.status() == WL_CONNECTED) {
		Serial.println(String()+"Connected to "+ssid1+" in "+String((millis()-ts)/1000)+"seconds");
		Serial.print("IP address: ");
		Serial.println(WiFi.localIP());
	} else Serial.println(String()+"Wifi connection to station failed "+ssid1+" in "+String((millis()-ts)/1000)+"seconds");
	return WiFi.status() == WL_CONNECTED;
}

bool WifiMan::connectStation2()
{

	wifimanpasswd=(*config)[RF(PASSFIELD)];
	wifimanssid=(*config)[RF(SSIDFIELD)];
	state=RF("AP_STA");
	//return connectST(wifimanssid.c_str(),wifimanpasswd.c_str());

	//	if(state==RF("none")){WiFi.mode(WIFI_STA);state=RF("STA");}
	//	if(state==RF("AP")){WiFi.mode(WIFI_AP_STA);state=RF("AP_STA");}
	WiFi.mode(WIFI_AP_STA);state=RF("AP_STA");
	// id AP_STA no need to change mode, just change network maybe or retry same
	println(GenString()+"WifiState:"+state);
	bool synced=false;
	//	GenString apname=config[RF(APNAMEFIELD)];

	if(wifimanssid.empty()) wifimanssid=RF("noconfigfile");
	if(wifimanpasswd.empty()) wifimanpasswd="";
	stopAP();
	WiFi.begin(wifimanssid.c_str(), wifimanpasswd.c_str());

	println(std::string()+RF("Connecting to wifi ssid : '")+wifimanssid.c_str()+RF("'"));
	m0=millis();
	print(RF("Connecting "));

	while ((millis()-m0)<timeout*1000 && WiFi.status() == WL_DISCONNECTED)
	{
		delay(500);
		print(RF("."));
		//	if(WiFi.status()==1) println(std::string()+"Ready "+to_string(WiFi.status()));
		//		if(WiFi.status()==6) println(std::string()+"Connecting "+to_string(WiFi.status()));
	}
	//bool retry=true;
	if(WiFi.status() == WL_CONNECTED){
		println(std::string()+RF("connected to wifi ")+wifimanssid+RF(" in ")+to_string(millis()-m0)+RF("ms"));
		print(RF("IP Address is: "));
		println(WiFi.localIP());
		//		stopAP()
		/ *
		println("pinger starting");

		syncer.pingSyncNTP(&wifilistener);
		println("will wait for NTP response");
		ntpresponse=false;
		while(!ntpresponse) delay(100);
		ntpresponse=false;
		synced=true;
		 * /
	} else{
		println(GenString() + RF("Failed to connect to wifi ssid :")+wifimanssid+" with password '"+wifimanpasswd+"'");
		if(WiFi.status() ==WL_IDLE_STATUS) println(RF("ssid not detected"));	//could stop STA here and restart every while (how long?) to see if ssid is back
		if(WiFi.status() ==WL_CONNECT_FAILED) {println(RF("password rejected"));/ *retry=false;* /}//could stop STA here until next change of password or next retry after timeout

		startAP(); 	//something about begin screwing the AP ?
	}

	//	if(retry) wifiTicker.attach_ms(1000,reconnectIfRequiredStat, this);
	println(GenString()+"WifiState:"+state);
	//mdiff=syncer.getSec()-CLOCK.getTimeMS()/1000;
	return synced;
}

/ *	WiFi.mode(WIFI_AP_STA);
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
