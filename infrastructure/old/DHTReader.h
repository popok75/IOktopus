#ifndef DHTREADER_H
#define DHTREADER_H


#include "../iodrivers/SensorReader.h"

#include <dht_nonblocking.h>

#define FTEMPLATE ".irom.text.dhtreader"

class DHTReader : public SensorReader
{
	DHT_nonblocking *dhtsensor=0;//= Sensirion(dataPin, clockPin);
	//FakeHTU htu = FakeHTU();	// could use a real htu too
	float temp, hum;
	bool readingtemp=true,readinghum=false;
	std::string tempname, humname;
	unsigned int pin;
	unsigned int type;
public:
	DHTReader(unsigned int pin0, unsigned int type0, std::string tempname0="", std::string humname0=""): pin(pin0){
		tempname=RF("Temperature");
		if(!tempname0.empty()) tempname=tempname0;
		humname=RF("Humidity");
		if(!humname0.empty()) humname=humname0;
	}
	~DHTReader(){if(dhtsensor) delete dhtsensor;}
	bool isNamed(std::string name){
		if(name==tempname || name==humname) return true;
		return false;};

	bool init() {
	 	//	if (!htu.begin()) {println("Couldn't find sensor HTU!");return false;}
		println(RF("DHTReader::init on pin:")+to_string(pin));
		dhtsensor=new DHT_nonblocking(pin,DHT_TYPE_22); //dataPin, clockPin
		//	if (!dhtsensor->begin()) {println("Couldn't find sensor HTU!");return false;}
		println(RF("DHTReader init with success!"));
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
		 float temperature;
		  float humidity;
		if(dhtsensor->measure( &temperature, &humidity ) == true)
		{
			Serial.println(String()+RF("DHT 22 sensor : temp= ") + String(temperature)+RF("C, humidity= ")+ String(humidity)+RF("%") );
			saveValue(tempname,to_stringWithPrecision(temperature,2));
			saveValue(humname,to_stringWithPrecision(humidity,2));
			temp=temperature;
			hum=humidity;
			//	print("Free heap sensor end :");	println(ESP.getFreeHeap(),DEC);
			return true;
		}
		if(dhtsensor->readfailure) {
			saveValue(tempname,RF(DISCONNECTED_STATE));
			saveValue(humname,RF(DISCONNECTED_STATE));

			return true;
		}
		return false;

	}

};
#endif


