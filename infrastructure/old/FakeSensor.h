#ifndef FAKESENSOR_H
#define FAKESENSOR_H

#include <time.h>

#include "../iodrivers/IOReader.h"

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.fakesensor"

/*
#define PSTR(s) (__extension__({static const char __c[] PROGMEM = (s); &__c[0];}))
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#define FF3(string_literal) (FPSTR(PSTR(string_literal)))
*/

// https://www.bountysource.com/issues/46510369-using-pstr-and-progmem-string-with-template-class-causes-section-type-conflict
// just rename template every few ticks
//#define FF3(string_literal) (reinterpret_cast<const __FlashStringHelper *>(((__extension__({static const char __c[] __attribute__((section(".irom.text.fakesensor"))) = ((string_literal)); &__c[0];})))))
//#define RF(x) String(FF3(x)).c_str()

//#define FTEMPLATE ".irom.text.fakesensor"



//#include "../CompatTicker.h"

/////////////////
class FakeValue {
public:
	std::string name;	//use GenString
	uint64_t timestamp=0;
	double val;
	double min, max,step;
	bool first=true;
	bool changed=false;

	FakeValue(std::string name0,double min0, double max0, double step0=0):name(name0), min(min0), max(max0){
		val=min+(max-min)*(rand()%100)/100.0;//+(rand()%1000)/100.0;
		if(step0==0) step=(max-min)/100;
		else step=step0;

	};
	void generate(){
		if(first) {srand (time(NULL));first=false;}

		float b=2*step*((rand()%100)/100.0-.5);
		if((val+b) <min) val=min;
		else if((val+b) >max) val=max;
		else val=val+b;

		timestamp=millis();//CLOCK.getTimeMS();
		changed=true;
	}
	bool hasChanged(){bool b=changed;changed=false;return b;}
};


//////////////////
class FakeSensor {
	std::vector<FakeValue*> fakevalues;
	bool reading=false;
	//TaskStruct *readts;
public:
	unsigned int readtimeout=300;
	uint64_t readtimestamp;

	bool empty(){return fakevalues.empty();}

	void addFakeValue(FakeValue*fval){fakevalues.push_back(fval);}
	void generateAll(){
		//		println("FakeSensor::postVal  ");
		for(FakeValue *fs : fakevalues){
			fs->generate();
		}
	}

	void read(){
		if(!reading) {reading =true;readtimestamp=millis();}
	};

	FakeValue * findByName(std::string nam){
		for(auto it: fakevalues){
			if(it->name==nam) return it;
		}
		return 0;
	}

	bool readAsync(std::string name,float &f,unsigned int ms=0){
		// find item with this name
		FakeValue *fv=findByName(name);
		if(!fv) return false;

		if(reading && (millis()-readtimestamp)>readtimeout) {generateAll();reading=false;}// simulate end of reading
		if(fv->timestamp){
			uint64_t dur=millis()-fv->timestamp;
			if(ms && dur<ms) {	// use already available value
				f=fv->val;
				reading=false;
				return true;
			}
		}
		read(); // start reading a new val
		return false;
	}
};

class FakeHTU {
public:
	FakeSensor fsensor;
	bool tempChanged=false, humChanged=false;
	bool begin(){
		if(fsensor.empty()){
			fsensor.addFakeValue(new FakeValue(RF("Temperature"),11,18));
			fsensor.addFakeValue(new FakeValue(RF("Humidity"),30,70));
			fsensor.readtimeout=75;//75ms to read both temp 25ms and humidity 50ms
		}
		return true;};
	bool readTemperatureAsync(float &f,unsigned int ms=0){
		bool b= fsensor.readAsync(RF("Temperature"),f,ms);
		tempChanged=fsensor.findByName(RF("Temperature"))->hasChanged();
		return b;
	};
	bool readHumidityAsync(float &f,unsigned int ms=0){
		bool b= fsensor.readAsync(RF("Humidity"),f,ms);
		humChanged=fsensor.findByName(RF("Humidity"))->hasChanged();
		return b;
	};
};

class FakeHTUSensorReader : public SensorReader
{
	FakeHTU htu = FakeHTU();	// could use a real htu too
	float temp, hum;
	bool readingtemp=true,readinghum=false;
	std::string tempname, humname;
public:
	FakeHTUSensorReader(std::string tempname0="", std::string humname0=""){
		tempname=RF("Temperature");
		if(!tempname0.empty()) tempname=tempname0;
		humname=RF("Humidity");
		if(!humname0.empty()) humname=humname0;
	}
	bool isNamed(std::string name){
		if(name==tempname || name==humname) return true;
		return false;};

	bool init() {
		if (!htu.begin()) {println(RF("Couldn't find sensor FakeHTUSensorReader!"));return false;}
		println(RF("FakeHTUSensorReader init with success!"));
		tickms=100;
		minafterread=0;

		//could emit one event only
		//		std::multimap<std::string,std::string>valmap={{"Humidity/type","input"},{"Humidity/unit","%"}};//,{"ts",to_string(CLOCK.getTimeMS())}};
		StringMapEvent arg({{RF("Humidity/type"),RF("input")},{RF("Humidity/unit"),RF("%")}});
		emit(&arg);
		//		valmap={{"Temperature/type","input"},{"Temperature/unit","°C"}};//,{"ts",to_string(CLOCK.getTimeMS())}};
		StringMapEvent arg2({{RF("Temperature/type"),RF("input")},{RF("Temperature/unit"),RF("°C")}});
		emit(&arg2);

		return true;
	};

	// HTU allow hold and no hold, take 50ms to read
	// for ESP8266 one process only, hold is no good, we use hold and come back every tick to check if the result changed
	bool sensorTick(){	// read temp, then hum
		//	Serial.println("HTU sensorTick");
		if(readingtemp){
			bool b=htu.readTemperatureAsync(temp,100);
			//	println("HTUSensorReader::sensorTick temp:");
			//	if(b) println("true"); else println("false");
			if(b && htu.tempChanged) {
				readingtemp=false;readinghum=true;
			}
			return false;
		}
		if(readinghum){
			bool b=htu.readHumidityAsync(hum,100);
			//	println("HTUSensorReader::sensorTick hum:");
			//	if(b) println("true"); else println("false");
			if(b && htu.humChanged) {
				readinghum=false;readingtemp=true;
				//	print("Free heap sensor :");	println(ESP.getFreeHeap(),DEC);
				saveValue(tempname,to_stringWithPrecision(temp,2));
				saveValue(humname,to_stringWithPrecision(hum,2));
				//	print("Free heap sensor end :");	println(ESP.getFreeHeap(),DEC);
				return true;
			}
		}
		return false;
	}

};

class FakeDSBSensorReader : public SensorReader
{
	FakeHTU htu;	// could use a real htu too
	float temp;
	bool readingtemp=true;
	std::string tempname;
public:
	FakeDSBSensorReader(unsigned int pin, std::string tempname0=""){
		tempname=RF("Temperature");
		if(!tempname0.empty()) tempname=tempname0;
	}
	bool isNamed(std::string name){
		if(name==tempname) return true;
		return false;
	};
	void rename(std::string name1){
			tempname=name1;
	}

	bool init() {
		if (!htu.begin()) {println(RF("Couldn't find sensor DSB!"));return false;}
		println(RF("FakeDSB init with success!"));
		tickms=100;
		minafterread=0;
		//		valmap={{"Temperature/type","input"},{"Temperature/unit","°C"}};//,{"ts",to_string(CLOCK.getTimeMS())}};
		StringMapEvent arg2({{tempname+RF("/type"),RF("input")},{tempname+RF("/unit"),RF("°C")}});
		emit(&arg2);

		return true;
	};

	// HTU allow hold and no hold, take 50ms to read
	// for ESP8266 one process only, hold is no good, we use hold and come back every tick to check if the result changed
	bool sensorTick(){	// read temp, then hum
/*		println(GenString()+"fakeDSB sensorTick"+to_string((uint64_t) this));
		println(GenString()+"fakeDSB sensorTick ticker "+to_string((uint64_t) &ticker)+" "+to_string((uint64_t) &ticker2));
		if(ticker.ts) println(GenString()+"fakeDSB::read ticker id :"+to_string( (ticker.ts->id)));
		if(ticker2.ts) println(GenString()+"fakeDSB::read ticker2 id : "+to_string( (ticker2.ts->id)));
*/
		bool b=htu.readTemperatureAsync(temp,100);
	//	println("fakeDSBSensorReader::sensorTick temp:");
	//	if(b) println("true"); else println("false");
		if(b && htu.tempChanged) {
			//	print("Free heap sensor :");	println(ESP.getFreeHeap(),DEC);
			saveValue(tempname,to_stringWithPrecision(temp,2));
			return true;
		}

		return false;
	}

};


#endif
