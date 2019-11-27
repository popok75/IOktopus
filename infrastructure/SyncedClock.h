#ifndef SYNCEDCLOCK
#define SYNCEDCLOCK

#include "CompatTicker.h"

#include "../datastruct/GenString.h"
#include "CompatPrint.h"

#define RESOLUTIONDEFINED 32
#define FRAGMENT 0.01


#include <time.h>

/*
class TestSyncer: public ClockListener
{
public:
	TestSyncer(){};
	virtual void clockresync(uint64_t diffms){
		println("TestSyncer:"+to_string(diffms));
	};
} testsyncer;
*/

class ClockListener{
public:
	virtual void clockresync(uint64_t diffms)=0;
};

class SyncedClock;
void tickstatic(SyncedClock *synclock);

class SyncedClock {	//instance total 76 bytes sram ?
	Ticker clockticker;  // 0 bytes sram ?

	bool clean=false;	//become true when we get a current timestamp by NTP or from client browser
	bool highpart=false, init=true;


	uint64_t boottime=0;		// boot time relative to now // resynced but not incremented
	uint64_t diff=0;		// difference between esp and real clock in ms // 8 bytes sram
	uint64_t halfstep;		// 8 bytes sram
	std::vector<ClockListener*> listeners;	// list of called when clock is resynced //should use (implement) GenList	// 12 bytes sram

public:
	SyncedClock(unsigned int resolution0=RESOLUTIONDEFINED){
		boottime=getMS();	//true for esp8266
		//Serial.begin(115200);
	//	println(GenString()+"SyncedClock::SyncedClock> boottime : "+to_string(boottime)+ " now:"+to_string(millis()));
#ifdef ESP8266BUILD
	//	boottime=0;
#endif
		halfstep=getHalfStep(resolution0);
		update();
	};

	uint64_t getHalfStep(unsigned int res){
		uint64_t ret=1;
		while(res>0){ret*=2;res--;}
		return ret/2;
	}



	void update(){

		uint64_t millin=mymillis(), rest=0;

		if(init){
			init=false;
			rest=halfstep-millin;
			if(millin>halfstep) {rest=halfstep-(millin-halfstep);highpart=true;}
		//	println(GenString()+"rest: "+to_string(rest));
			rest+=halfstep*FRAGMENT;
			rest+=1;
		//	println(GenString()+"rest2: "+to_string(rest));
			clockticker.once_ms(rest, tickstatic, this);
			println(RF("SyncedClock init ok"));
			return;

		} else if(millin<halfstep && highpart){
		//	println(GenString()+"millin: "+to_string(millin));
		//	println(GenString()+"diff: "+to_string(diff));
			highpart=false;
			diff+=halfstep*2;
			rest=halfstep-(millin)+halfstep*FRAGMENT+1;
			clockticker.once_ms(rest, tickstatic, this);
			return;

		} else if(millin>halfstep && !highpart){
		//	println(GenString()+"millin: "+to_string(millin));
		//	println(GenString()+"diff: "+to_string(diff));
			highpart=true;
			rest=halfstep-(millin-halfstep)+halfstep*FRAGMENT+1;
			clockticker.once_ms(rest, tickstatic, this);
			return;
		}

	}

	void tick(){
		//		tickercall=true;
		update();
		//		tickercall=false;
	};
private:
	uint64_t mymillis(){
		uint64_t ms=millis(), re=halfstep*2-1;
		ms=ms&re;
		return ms;
	};
public:
//	uint64_t save=0;
	uint64_t getMS(){
		update();	//if needed
		uint64_t full=diff+mymillis();
	//	println(GenString()+"Clock::getBoottime> getMS full: "+to_string(full)+ " mymillis():"+to_string(mymillis()));
	//	if(save>full){	println(GenString()+"problem here clock going backward");	}
	//	save=full;

//		uint64_t now=millis64();
//		std::cout << " millis64:" << to_string(now) << " vs. clock : "<< full <<std::endl;
		return full;

	}

	uint64_t getDiff(){
			update();	//if needed
			return diff;
	}
	uint64_t getBoottime(){
			update();	//if needed
//			println(GenString()+"Clock::getBoottime> boottime raw: "+to_string(boottime)+ " final:"+to_string(boottime));
			return diff;
	}

	uint64_t getSec(){return getMS()/1000;}

	void resyncSec(uint64_t now, bool clean1=true){resyncMS(now*1000,clean1);}

	void resyncMS(uint64_t now, bool clean1=true){
		uint64_t mym=mymillis();
//		println(GenString()+"Clock::resyncMS> mym: "+to_string(mym));
//		println(GenString()+"Clock::resyncMS> now: "+to_string(now));
		uint64_t diffms=now-getMS();
		boottime+=diffms; //
		diff=(now-mym);

		clean=clean1;
		println(GenString()+RF("Clock::resyncMS> new clock: ")+to_string(getMS()));

		uint32_t sec=getSec();
		println(ctime((time_t*)(&sec)));
		for(ClockListener* s:listeners){
			 s->clockresync(diffms);
		}
	}


	void addListener(ClockListener *cl){
		listeners.push_back(cl);
	}

	GenString getNowAsString(){
		uint32_t sec=getSec();
		GenString str(ctime((time_t*)(&sec)));
		while(str[str.length()-1]=='\n' || str[str.length()-1]=='\r') str=str.substr(0,str.length()-1);
		return str;
	}

	// - first, start a timer until highest bit on+fragment
	// - at timer, update
	// if inconsistent ->if highpart increment diff, change highpart, cancel any timer, start a new timer until highest bit off+fragment

	// getMS64 : - if highpart inconsistent update, return diff*1000+millis
	//	- inconsistent : highpart is 1 and highest millis() bit is 0 or the opposite

} CLOCK32(32);

void tickstatic(SyncedClock *autoclock){
	//	Serial.println( "required" );
	static_cast<SyncedClock *>(autoclock)->tick();
}

#endif


