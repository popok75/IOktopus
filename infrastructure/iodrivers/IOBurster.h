#ifndef SENSORREADER_H
#define SENSORREADER_H


/* IOReader is a base class with 2 timers : autoreading every x seconds, sending ticks to sensor every y ms
 			// p.s. IOReader should not have so many functions and should be restricted to timers and reading
 * */

#include <string>	// should we replace this by GenString ? see saveValue() below


#undef FTEMPLATE
#define FTEMPLATE ".irom.text.ioburster"

#define DISCONNECTED_STATE "disconnected"
#define INIT_STATE "init"

class IOBursterListener {
public:
	virtual bool tick()=0;
};

class IOBurster
{
private:

	Ticker ticker, ticker2;
	bool reading=false,ready=false;
	IOBursterListener* listener=0;

public:
	uint32_t reqms=0, tickms=10, minafterread=1000;
	IOBurster(IOBursterListener*listener1);
	void tick();	//internal

	void start(unsigned int sec);
	void startMS(unsigned int ms);
	bool burst();
	bool once();
	void stop();

	//if sensorTick return true, the ticker2 is stopped, why ? problem !!!!
	// implicit stopping when finished reading ?
};


#include "../CompatPrint.h"


IOBurster::IOBurster(IOBursterListener*listener1):listener(listener1)
{
	//	println(RF("Constructor IOReader"));
	//	ticker2.debug=true;
};



// Following funcs would they work with multiple instances ?

void startread(IOBurster *reader){
	//	static_cast<IOReader *>
	(reader)->burst();
}

void IOBurster::stop(){
	//	println(GenString()+"IOReader::readtick detaching "+to_string((uint64_t) this));
	//	ticker2.debug=false;
	ticker2.detach();	// but reading is not modified here !?
	//	ticker2.debug=true;

	//	println(GenString()+"IOReader::readtick detached "+to_string((uint64_t) this));
	//			println("detached " );
	reading=false;ready=true;

}

void IOBurster::tick(){
	if(listener) listener->tick();
}

void readtick(IOBurster *reader){
	//println("readtick");
	//	println(GenString()+"IOReader::readtick "+to_string((uint64_t) reader));
	static uint32_t measurement_timestamp = 0;
	//	println(GenString()+"IOReader::readtick millis"+String(millis())+" mts"+String(measurement_timestamp)+" minafter"+String(reader->minafterread));
	if(millis() - measurement_timestamp > reader->minafterread )	// prevent asking another value for 1s
	{
		reader->tick();
		measurement_timestamp = millis();
		//		print("IOReader::readtick post sensortick ");
		//		if(b) println("true "); else println("false ");
	}
};


void IOBurster::start(unsigned int sec){
	//	println(std::string()+RF("IOReader::autoread: ")+to_string(sec));
	startMS(sec*1000);
}

void IOBurster::startMS(unsigned int ms){
	//	println(std::string()+"IOReader::autoreadMS: "+String(sec).c_str());
	if(ms){
		ticker.attach_ms(ms,startread,this);
	} else {
		ticker.detach();
	}
}



bool IOBurster::once(){
	//	println(GenString()+"IOReader::read "+to_string((uint64_t) this));
	/*	println(GenString()+"IOReader::read ticker "+to_string((uint64_t) &ticker)+" "+to_string((uint64_t) &ticker2));
	println(GenString()+"IOReader::read ticker ts :"+to_string((uint64_t) (ticker.ts))+" "+to_string((uint64_t) (ticker2.ts)));
	if(ticker.ts) println(GenString()+"IOReader::read ticker id :"+to_string( (ticker.ts->id)));
	if(ticker2.ts) println(GenString()+"IOReader::read ticker2 id : "+to_string( (ticker2.ts->id)));
	 */	if(!reading) {//println("!reading");
		 reqms=millis();
		 ticker2.attach_ms(tickms, readtick, this);	// should get the tick ms from the driver or else work always full speed
		 //		println(GenString()+"ticker2 attached "+to_string(tickms));
		 reading=true;
		 return true;
	 } else {	// happen if reading is still true when we ask next value, if we stop ticker2 without saving before
		 //		println("IOReader::read: sensor already in reading state");
		 return false;
	 }
}



#endif
