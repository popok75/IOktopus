#ifndef NTPSYNCER_H
#define NTPSYNCER_H
#include <Ticker.h>

///////////////////////////////////////
#include <ESP8266WiFi.h>       // https://github.com/esp8266/Arduino
#include <user_interface.h>  // https://github.com/esp8266/Arduino
#include <ping.h>            // https://github.com/esp8266/Arduino

#define IPTEST "8.8.8.8"
#define NTPSERVER1 "gr.pool.ntp.org"
#define NTPSERVER2 "time.windows.com"
#define NTPSERVER3 "de.pool.ntp.org"

#define PINGCOUNT 3
#define PINGINTERVAL 1
//const uint8_t pingCount = 3;     // number off Ping repetition
//const uint8_t pingInterval = 1;  // Ping repetition every n sec




class PingListener {
public:
	virtual void response(bool)=0;
};
struct ping_option_plus : public ping_option {PingListener *listener;};

class Pinger {

	struct ping_option_plus pOpt;

	static void pingRecv(void *arg, void *pdata) {  // Pong callback function
		struct ping_option_plus *pingOpt = (struct ping_option_plus *)arg;
		struct ping_resp *pingResp = (struct  ping_resp *)pdata;

		//Pinger *pingerinst=pingOpt->pinger;
		PingListener *listener=pingOpt->listener;
		if (pingResp->ping_err == -1) {Serial.println( RF("No Pong (device OFFline)") );listener->response(false);}//if(pingerinst->cbfunc && !pingerinst->calledcb) pingerinst->cbfunc(false);}
		else  {

	 		Serial.println(RF("Pinger::pingRecv: bytes = ") +String(pingResp->bytes) + RF(", time = ") +String( pingResp->resp_time)+ RF("ms") );
			listener->response(true);
			//	if(pingerinst->cbfunc && !pingerinst->calledcb) pingerinst->cbfunc(true);
		}
		// pingerinst->calledcb=true;
	}

public:

	Pinger(){};

	void doPing(const char *targetIpAddress, PingListener*listener0, int pcount=PINGCOUNT) {   // init and start Ping


		//	Serial.println(String()+"Pinger::do ping start "+targetIpAddress);
		struct ping_option_plus *pingOpt = &pOpt;
		pingOpt->count = PINGCOUNT;
		pingOpt->coarse_time = PINGINTERVAL;
		pingOpt->ip = ipaddr_addr(targetIpAddress);
		pingOpt->listener = listener0;

		ping_regist_recv(pingOpt, pingRecv);  // Pong callback function 'pingRecv'
		// ping_regist_sent(pingOpt, pingSent);  // Ping finished callback function 'pingSent'
//		calledcb=false;

		ping_start(pingOpt);  // start Ping
 		Serial.println(String()+"Pinger::do ping started ping "+targetIpAddress);
	}
	void checkIfOnline(PingListener*listener0){
	 	Serial.println(String()+"Pinger::checkIfOnline() start ");
		doPing(RF(IPTEST),listener0,1);
	};
};






#include <time.h>

#define RECENTSECTS 1521164740

#define NTPTIMEOUT 8000
#define NTPCHECK 1000

class NTPSyncer;
class NTPListener : public PingListener{
public:
	NTPSyncer *mysyncer=0;
	NTPListener(NTPSyncer *mysyncer0):mysyncer(mysyncer0){};
	void response(bool online);//{mysyncer->sonlineCB(online,this);pingUpdate(online);};
	virtual void pingUpdate(bool online){};
	virtual void ntpUpdate(uint32_t,bool)=0;
};

class NTPSyncer{	// check if onling then ask a ntp update, wait for update or timeout
	Ticker ntpticker,ntptimeoutticker;
	uint32_t m0,diff;
//	void (*func)(uint32_t);
//	void (*func2)(uint32_t,bool);
	Pinger ntppinger;

//	String s1,s2,s3;

public:
	bool synced=false;

	uint32_t getSec(){
		time_t sec = 0;
		time(&sec);
		return (uint32_t)sec;
	}
	void tick(NTPListener *ntplistener){
		uint32_t sec=getSec();
	//	println(std::string()+"(sec-millis()/1000): "+to_string(sec-millis()/1000)+" diff:"+to_string(diff));
		if((sec-millis()/1000)!=diff) {
		//	println(std::string()+"(sec-millis()/1000)!=diff "+to_string(sec-millis()/1000)+" "+to_string(diff));
	//		Serial.print(RF("NTP Synced in "));
			Serial.print (millis()-m0);
			Serial.print(RF("ms to "));
			Serial.print(sec);
			Serial.print(RF(" - "));
			Serial.println(ctime((time_t*)(&sec)));
			diff=sec-millis()/1000;
			//ntpticker.detach();
			ntptimeoutticker.detach();
			ntplistener->ntpUpdate(sec,true);
		}
	};

	static void tstat(NTPListener *ntplistener){ntplistener->mysyncer->tick(ntplistener);}
	static void timeoutstat(NTPListener *ntplistener){
		Serial.println(RF("NTP timout - no internet or already synced ?"));
		//syncer->ntpticker.detach();
		ntplistener->ntpUpdate(0,true);	//timeout call cb with time = 0
	};
	void stopBGTicker(){ntpticker.detach();};
	void start(NTPListener *ntplistener){

		m0=millis();
	//	println(GenString()+"NTPSyncer::start "+ to_string((uint64_t)m0));
		time_t sec = 0;
		time(&sec);
		diff=sec-m0/1000;

//		configTime(0, 0, RF(NTPSERVER1), RF(NTPSERVER2), RF(NTPSERVER3));	// ask utc time
//		s1=RF(NTPSERVER1);s2=RF(NTPSERVER2);s3=RF(NTPSERVER3);configTime(0, 0, s1.c_str(), s2.c_str(), s3.c_str());	// works but expensive
		configTime(0, 0, (NTPSERVER1), (NTPSERVER2), (NTPSERVER3));	// after 2.5 cannot be RFised, they need to be real CONST char * (probably they are used async after the function is called)

		ntpticker.detach();

		ntpticker.attach_ms(NTPCHECK, tstat, ntplistener);

		ntptimeoutticker.detach();

		ntptimeoutticker.once_ms(NTPTIMEOUT, timeoutstat, ntplistener);

	};
	void syncNTP(NTPListener *ntplistener){//void (*func0)(uint32_t)){
	//	func=func0;
		start(ntplistener);
	}

	void sonlineCB(bool online,NTPListener *ntplistener){
		if(online) start(ntplistener);
		else ntplistener->ntpUpdate(0,false);
	}
	void  pingSyncNTP(NTPListener *ntplistener){
		ntppinger.checkIfOnline(ntplistener);
	}
};

void NTPListener::response(bool online){
	println(GenString()+"NTPListener::response "+ to_string((uint64_t)online));
	if(!mysyncer->synced) {	//now is called at first ping // should call callback at first positive or last negative ping
		mysyncer->sonlineCB(online,this);mysyncer->synced=true;
	}
	pingUpdate(online);
}
#endif



