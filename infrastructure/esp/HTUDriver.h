#ifndef HTUDRIVER_H
#define HTUDRIVER_H



#include <AsyncHTU21D.h>	// custom driver from sparkfunHTU21D modified to get async and the error back
#include "../iodrivers/IODriver.h"
#undef FTEMPLATE
#define FTEMPLATE ".irom.text.htudriver"


class HTUDriver : public TempRHDriver	// HTU21D fully hot pluggable
{
	HTU21D *htu=0;//= Sensirion(dataPin, clockPin);
	//FakeHTU htu = FakeHTU();	// could use a real htu too

	bool readingtemp=true,readinghum=false;

	bool firstread=true;
	const unsigned int timeout=2000;
	uint32_t reqts=0;
public:
	HTUDriver(unsigned int num0, std::vector<unsigned int >pins0) : TempRHDriver(num0,pins0){}
	~HTUDriver(){if(htu) delete htu;}



	bool init() {
		if(pins.size()==2) htu=new HTU21D(pins[1], pins[0]); //dataPin, clockPin
		//if (!htu->begin()) {println(RF("Couldn't find sensor HTU!"));return false;}
		if (!htu->begin()) {println(RF("Couldn't find sensor HTU!"));}
		else println(RF("HTUReader init with success!"));

		return true;
	};

	// HTU allow hold and no hold, take 50ms to read
	// for ESP8266 one process only, hold is no good, we use hold and come back every tick to check if the result changed
	bool sensorTick(){	// read temp, then hum
	//		Serial.println("HTU sensorTick");
		uint32_t now=millis();
		if (firstread) {
			reqts=now;firstread=false;
		} else if((now-reqts)>timeout){
			disconnected=true;
			firstread=true;
			return true;
		}
		if(readingtemp){
			bool b=htu->readTemperatureAsync(temp,100);

			if(htu->readTimeout){
				disconnected=true;
				firstread=true;
				return true;
			}
			if (b) disconnected=false;
			if(b && htu->tempChanged) {
				readingtemp=false;readinghum=true;
			}
			return false;
		}
		if(readinghum){
			bool b=htu->readHumidityAsync(hum,100);
 			if(htu->readTimeout){
 				disconnected=true;
				firstread=true;
				return true;
			}
 			if (b) disconnected=false;
			if(b && htu->humChanged) {
				readinghum=false;readingtemp=true;
				firstread=true;
				return true;
			}
		}
		return false;
	}

};

#endif

