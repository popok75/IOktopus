#ifndef HTUREADER_H
#define HTUREADER_H



#include "../iodrivers/SensorReader.h"

#include <AsyncHTU21D.h>	// custom driver from sparkfunHTU21D modified to get async and the error back

#define FTEMPLATE ".irom.text.htureader"


class HTUReader : public SensorReader	// HTU21D fully hot pluggable
{
	HTU21D *htu=0;//= Sensirion(dataPin, clockPin);
	//FakeHTU htu = FakeHTU();	// could use a real htu too
	float temp, hum;
	bool readingtemp=true,readinghum=false;
	std::string tempname, humname;
	std::vector<unsigned int >pins;
	bool firstread=true;
	const unsigned int timeout=2000;
	uint32_t reqts=0;
public:
	HTUReader(std::vector<unsigned int >pins0, std::string tempname0="", std::string humname0=""): pins(pins0) {
		tempname=RF("Temperature");
		if(!tempname0.empty()) tempname=tempname0;
		humname=RF("Humidity");
		if(!humname0.empty()) humname=humname0;

	}
	~HTUReader(){if(htu) delete htu;}
	bool isNamed(std::string name){
		if(name==tempname || name==humname) return true;
		return false;};

	bool init() {
		if(pins.size()==2) htu=new HTU21D(pins[1], pins[0]); //dataPin, clockPin
		//if (!htu->begin()) {println(RF("Couldn't find sensor HTU!"));return false;}
		if (!htu->begin()) {println(RF("Couldn't find sensor HTU!"));}
		else println(RF("HTUReader init with success!"));
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
		uint32_t now=millis();
		if (firstread) {
			reqts=now;firstread=false;
		} else if((now-reqts)>timeout){
			saveValue(tempname,RF(DISCONNECTED_STATE));
			saveValue(humname,RF(DISCONNECTED_STATE));
		//	if (!htu->begin()) {println(RF("Couldn't find sensor HTU!"));}	else println(RF("HTUReader init with success!"));
			firstread=true;
			return true;
		}
		if(readingtemp){
			bool b=htu->readTemperatureAsync(temp,100);
	//		println("HTUSensorReader::sensorTick temp:");
	//		if(b) println("true"); else println("false");
	//		if(htu->readTimeout) println("true"); else println("false");
			if(//!b &&
					htu->readTimeout){
				saveValue(tempname,RF(DISCONNECTED_STATE));
				saveValue(humname,RF(DISCONNECTED_STATE));
	//			htu->begin();
	 	//		if (!htu->begin()) {println(RF("Couldn't find sensor HTU!"));}	else println(RF("HTUReader init with success!"));
				firstread=true;
				return true;
			}
			if(b && htu->tempChanged) {
				readingtemp=false;readinghum=true;
			}
			return false;
		}
		if(readinghum){
			bool b=htu->readHumidityAsync(hum,100);
	//	 	println("HTUSensorReader::sensorTick hum:");
	//	 	if(b) println("true"); else println("false");
	//	 	if(htu->readTimeout) println("true"); else println("false");
			if(//!b &&
					htu->readTimeout){
				saveValue(tempname,RF(DISCONNECTED_STATE));
				saveValue(humname,RF(DISCONNECTED_STATE));
//				htu->begin();
 		//		if (!htu->begin()) {println(RF("Couldn't find sensor HTU!"));}	else println(RF("HTUReader init with success!"));
				firstread=true;
				return true;
			}
			if(b && htu->humChanged) {
				readinghum=false;readingtemp=true;
				//	print("Free heap sensor :");	println(ESP.getFreeHeap(),DEC);
				saveValue(tempname,to_stringWithPrecision(temp,2));
				saveValue(humname,to_stringWithPrecision(hum,2));
				//	print("Free heap sensor end :");	println(ESP.getFreeHeap(),DEC);
				firstread=true;
				return true;
			}
		}
		return false;
	}

};

#endif

