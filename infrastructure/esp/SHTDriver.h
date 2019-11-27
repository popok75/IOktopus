#ifndef SHTDRIVER_H
#define SHTDRIVER_H



//#include "../iodrivers/SensorReader.h"

#include <Sensirion.h>
const float _D1  = -40.1; // from Sensirion.cpp
#undef FTEMPLATE
#define FTEMPLATE ".irom.text.shtdriver"

bool readSensor(float &temp, float &hum, float &dew, Sensirion *sht);

// sht15 does reconnect clock/data, ground is not required
// but vcc reconnect cause hang require clock/data reconnect to rework
// a start with no vcc never allow to work, a start with no clock/data/ground is no problem

class SHTDriver : public TempRHDriver	// hot pluggable 100%
{
	Sensirion *sht=0;//= Sensirion(dataPin, clockPin);
	float dew;
	bool readingtemp=true,readinghum=false;

	uint32_t reqts;
	unsigned int timeout=4000;

public:
	SHTDriver(unsigned int num0, std::vector<unsigned int >pins0) : TempRHDriver(num0,pins0){}

	~SHTDriver(){if(sht) delete sht;}


	bool init() {
	 	if(pins.size()==2) sht=new Sensirion(pins[1], pins[0]); //dataPin, clockPin
		else return false;
		println(GenString()+RF("SHTDriver init with success with pins data:")+to_string(pins[1])+RF(", clock: ")+to_string(pins[0]));

		return true;
	};

	// HTU allow hold and no hold, take 50ms to read
	// for ESP8266 one process only, hold is no good, we use hold and come back every tick to check if the result changed
	bool sensorTick(){	// read temp, then hum
		//	Serial.println("HTU sensorTick");
		float dewpoint=-1;
		bool b=readSensor();
		uint32_t now=millis();
		//unsigned int timeout=4000;
		//Serial.println(String()+"sensorTick "+String());
		//	Serial.println(String()+"reqts:"+to_string((reqts)).c_str());
		//	Serial.println(String()+"now:"+to_string((now)).c_str());
		if((now-reqts)>timeout || readerror || temp==_D1){
			//		Serial.println(String()+"timeout:"+to_string((now-reqts)).c_str());
			//		Serial.println(String()+"readerror:"+String(readerror));
			Serial.print(RF("\nError. Readerror : ")+String(readerror)+RF(", timeout:")+to_string((now-reqts)).c_str()+RF(", ")+String(temp));
			disconnected=true;
			sht->reset();
			temp=0;
			readingtemp2=false;
			return true;
		}
		if(b) {
			Serial.print(RF("\nTemperature: "));
			Serial.print(temp);
			Serial.print(RF(" C, Humidity: "));
			Serial.print(hum);
			Serial.print(RF(" %, Dewpoint: "));
			Serial.print(dewpoint);
			Serial.println(RF(" C"));
			disconnected=false;
		} //else Serial.print(RF("."));
		return b;
	}
#undef FTEMPLATE
#define FTEMPLATE ".irom.text.shtdriver2"
	void logError(byte error) {
		switch (error) {
		case S_Err_NoACK:Serial.println(RF("Error: No response (ACK) received from sensor!"));break;
		case S_Err_CRC:Serial.println(RF("Error: CRC mismatch!"));break;
		case S_Err_TO:Serial.println(RF("Error: Measurement timeout!"));break;
		default:Serial.println(RF("Unknown error received!"));break;
		}
	}
	bool readingtemp2=false, readinghum2=false;
	//	float stemp, shum, sdew;
	uint16_t rawData;
	bool readerror=false;
	bool readSensor(){
		readerror=false;
	 		if(!readingtemp2 && !readinghum2){
 			byte e;
			if((e=sht->meas(TEMP, &rawData, NONBLOCK))) {	Serial.println(RF("Error reading temperature"));logError(e);readerror=true;return false;};
 			readingtemp2=true;
			reqts=millis();
		}
		if(readingtemp2 && sht->measRdy()){
	 		readingtemp2=false;
			temp=sht->calcTemp(rawData);
			byte e;
			if((e=sht->meas(HUMI, &rawData, NONBLOCK))) {Serial.println(RF("Error reading humidity"));logError(e);readerror=true;return false;};
			readinghum2=true;
		}
		if(readinghum2 && sht->measRdy()){
	 		readinghum2=false;
			hum= sht->calcHumi(rawData, temp); // Convert raw sensor data
			//    dew = log(shum/100) ;
			dew= sht->calcDewpoint(hum, temp);
			return true;
		}

		return false;
	};
};

#endif

/*
 * Query a SHT10 temperature and humidity sensor
 *
 * A simple example that queries the sensor every 5 seconds
 * and communicates the result over a serial connection.
 * Error handling is omitted in this example.
 */


//const uint8_t dataPin  =  D3; // yellow
//const uint8_t clockPin =  D0; //green











