#ifndef SENSORREADER_H
#define SENSORREADER_H

#include <string>	// should we replace this by GenString ? see saveValue() below



#include "../events/DefaultEventEmitter.h"

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.sensorreader"

#define DISCONNECTED_STATE "disconnected"
#define INIT_STATE "init"

class IOReader : public DefaultEventEmitter
{
public:

	uint32_t mstimestamp=0;
	uint32_t reqms=0,tickms=10,minafterread=1000;

	Ticker ticker, ticker2;

	IOReader(void);
	virtual ~IOReader(void){};

	void loop();
	void autoread(unsigned int sec);
	void autoreadMS(unsigned int ms);
	virtual bool read();
	void saveValue(std::string name,std::string value,uint64_t tsms);
//	uint32_t getValues();

	virtual bool sensorTick()=0;	//if sensorTick return true, the autoread is stopped, why ?
	virtual bool isNamed(std::string name)=0;
	virtual void rename(std::string name){};
	virtual bool init(){return true;};
	virtual bool sharedPins(GenString newdevmodel){return false;};	//if true type is involved
	void save(uint64_t tsms);

protected:
	bool reading=false,ready=false;

};


#include "../CompatPrint.h"


IOReader::IOReader(void)
{
	println(RF("Constructor IOReader"));

};



// Following funcs would they work with multiple instances ?

void startread(IOReader *reader){
//	static_cast<IOReader *>
	(reader)->read();
}

void readtick(IOReader *reader){
 	//println("readtick");
//	println(GenString()+"IOReader::readtick "+to_string((uint64_t) reader));
	static uint32_t measurement_timestamp = 0;
//	Serial.println(String()+"IOReader::readtick millis"+String(millis())+" mts"+String(measurement_timestamp)+" minafter"+String(reader->minafterread));
	if(millis() - measurement_timestamp > reader->minafterread )	// prevent asking another value for 1s
		{if(  reader->sensorTick() == true )
		{
		//	println("");
			reader->ticker2.detach();
//			Serial.println("detached" );
			measurement_timestamp = millis();
		} else print(".");}
};
/*
uint32_t IOReader::getValues(){

	return mstimestamp;
}
*/

void IOReader::autoread(unsigned int sec){
	println(std::string()+RF("IOReader::autoread: ")+to_string(sec));
	if(sec){
 		ticker.attach_ms(sec*1000,startread,this);
		startread(this);
	} else {
 		ticker.detach();
	}
}

void IOReader::autoreadMS(unsigned int ms){
//	println(std::string()+"IOReader::autoreadMS: "+String(sec).c_str());
	if(ms){
		ticker.attach_ms(ms,startread,this);
	} else {
		ticker.detach();
	}
}


void IOReader::save(uint64_t tsms){
		if(tsms==0) mstimestamp= millis();
		else mstimestamp=tsms;

		reading=false;ready=true;	// save only when finished reading or change this line
}

bool IOReader::read(){
/*	println(GenString()+"IOReader::read "+to_string((uint64_t) this));
	println(GenString()+"IOReader::read ticker "+to_string((uint64_t) &ticker)+" "+to_string((uint64_t) &ticker2));
	println(GenString()+"IOReader::read ticker ts :"+to_string((uint64_t) (ticker.ts))+" "+to_string((uint64_t) (ticker2.ts)));
	if(ticker.ts) println(GenString()+"IOReader::read ticker id :"+to_string( (ticker.ts->id)));
	if(ticker2.ts) println(GenString()+"IOReader::read ticker2 id : "+to_string( (ticker2.ts->id)));
*/	if(!reading) {//println("!reading");
		reqms=millis();
		ticker2.attach_ms(tickms, readtick, this);	// should get the tick ms from the driver or else work always full speed
	//	println(GenString()+"ticker2 attached "+to_string(tickms));
		reading=true;
		return true;
	} else {
//		println("reading");
		return false;
	}
}



#endif
