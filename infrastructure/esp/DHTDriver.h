#ifndef DHTREADER_H
#define DHTREADER_H


#include "../iodrivers/SensorDriver.h"

#include <dht_nonblocking.h>
#undef FTEMPLATE
#define FTEMPLATE ".irom.text.dhtreader"

class DHTDriver : public TempRHDriver
{
	DHT_nonblocking *dhtsensor=0;//= Sensirion(dataPin, clockPin);


public:
	DHTDriver(unsigned int num0, std::vector<unsigned int> pins0):TempRHDriver(num0,pins0){}
	~DHTDriver(){if(dhtsensor) delete dhtsensor;}


	bool init() {
		if(pins.empty()) return false;
		println(RF("DHTReader::init on pin:")+to_string(pins[0]));
		dhtsensor=new DHT_nonblocking(pins[0],DHT_TYPE_22); //dataPin, clockPin
		println(RF("DHTReader init with success!"));
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
		 	temp=temperature;
			hum=humidity;
			disconnected=false;
			//	print("Free heap sensor end :");	println(ESP.getFreeHeap(),DEC);
			return true;
		}
		if(dhtsensor->readfailure) {
			disconnected=true;
			return true;
		}
		return false;

	}

};
#endif


