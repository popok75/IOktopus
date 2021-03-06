#ifndef DSBREADER_H
#define DSBREADER_H


// DS18B20 if we hot disconnect : data/ground/vcc -> -127C, data-vcc resistor -> timeout (no conversion)



//#include <string>
//#include <time.h>

#include "../iodrivers/IOReader.h"

#define FTEMPLATE ".irom.text.dsbreader"
//#define FFDSB(string_literal) (reinterpret_cast<const __FlashStringHelper *>(((__extension__({static const char __c[] __attribute__((section(".irom.text.dsbreader"))) = ((string_literal)); &__c[0];})))))
//#define RF(x) String(FFDSB(x)).c_str()



void SetupDS18B20();
void TempLoop(long);

// create static DSBbus first time or else use existing

static unsigned int idgenerate=0;

class DSBBus;

DSBBus *getDSBBus(unsigned int pin);

class DSBReader : public SensorReader
{
	float temp;
	bool readingtemp=true;
	std::string tempname;
	uint32_t lastts=0;
	DSBBus *dsbbus;
	unsigned int id;
	unsigned int pin;
public:
	DSBReader(unsigned int pin0, std::string tempname0=std::string()): tempname(tempname0),pin(pin0){
		tempname=RF("Temperature");
		if(!tempname0.empty()) tempname=tempname0;
		id=idgenerate++;
	}

	bool isNamed(std::string name){
		if(name==tempname) return true;
		return false;};
	void rename(std::string name1){
		tempname=name1;
	}
	bool init();

	bool sensorTick();

};






#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_MAX_DEV 15 //The maximum number of devices

String GetAddressToString(DeviceAddress deviceAddress);



class DSBBus {

	struct DeviceInfo{
		DeviceAddress address={0,0,0,0,0,0,0,0};
		float lastvalue=0;//DEVICE_DISCONNECTED_C;
		long lastTS=0;
	};

	std::vector<DeviceInfo> devices;

	const int durationTemp = 300; //The frequency of temperature measurement

	unsigned int pin;
	OneWire oneWire;
	DallasTemperature DS18B20;

public:
	bool reading=false,redetect=true;bool alldisconnected=true;
	long reqts=0; //The last measurement timestamp

	const int timeout = 3000; //The frequency of temperature measurement

	DSBBus(unsigned int pin0):oneWire(pin0),DS18B20(&oneWire){
		//	for( float &f : tempDev) f=DEVICE_DISCONNECTED_C ;	// fill with DEVICE_DISCONNECTED_C  in case of
		SetupDS18B20();
	}
	bool read(){
		Serial.println("DSBBus::read ");
		if(reading) return false;
		if(devices.empty()){redetectSensors();if(devices.empty()) return false;}

		DS18B20.setWaitForConversion(false); //No waiting for measurement
		DS18B20.requestTemperatures(); //Initiate the temperature measurement
		reqts=millis();
		redetect=true;
		reading =true;
		Serial.println("DSBBus::read ended");
		return true;
	}


	float getTemperature(unsigned int index){
		if(index>=devices.size()) return DEVICE_DISCONNECTED_C;
		return devices[index].lastvalue;
	};



	long getTimestamp(unsigned int index){
		if(index>=devices.size()) return 0;
		return devices[index].lastTS;
	};

	unsigned int count(){return devices.size();}

	void tick(){
		if(!reading) return;
		//	Serial.println("DSBBus::sensorTick");
		unsigned long now=millis();

		if(!alldisconnected && (now-reqts)>timeout){
			for(int i=0; i<devices.size(); i++){
				devices[i].lastvalue = DEVICE_DISCONNECTED_C; //Save the measured value to the array
				devices[i].lastTS = now;  //Remember the last time measurement
			}
			alldisconnected=true;
			reading=false;
			Serial.println(RF("Sensors DS18B20 timed-out !"));
		}

		if((now-reqts)>durationTemp){
			if(DS18B20.isConversionComplete()){
				alldisconnected=false;
				Serial.println(RF("conversion took : ")+String(now - reqts));
				for(int i=0; i<devices.size(); i++){
					float tempC = DS18B20.getTempC( devices[i].address ); //Measuring temperature in Celsius
					devices[i].lastTS = now;//Remember the last time measurement
					if(tempC==DEVICE_DISCONNECTED_C) {
						Serial.println(String()+RF("Sensor DS18B20 ")+i+RF(" with address ")+GetAddressToString(devices[i].address)+RF(" disconnected !!"));
						devices[i].lastvalue = DEVICE_DISCONNECTED_C;
						continue;
					}
					devices[i].lastvalue = tempC; //Save the measured value to the array
					Serial.println(String()+RF("Sensor DS18B20 ")+String(i)+RF(" with address ")+GetAddressToString(devices[i].address)+RF(" measured temp : ")+String(tempC)+RF("C"));
				}
				reading=false;
			}

		}
		if(redetect){redetectSensors();redetect=false;}
	}


	void updateDevices(DeviceAddress devaddr){
		bool found=false;
		//	unsigned int i;
		for(auto dev:devices){
			if(GetAddressToString(dev.address)==GetAddressToString(devaddr)){
				//			Serial.println(RF("Found known device ")+String(i)+RF(" with address: ") + GetAddressToString(devaddr));
				found=true; break;
			}
			//i++;
		}
		if(!found){
			alldisconnected=false;
			Serial.println(RF("Found new DS18B20 device ")+String(devices.size())+RF(" with address: ") + GetAddressToString(devaddr));
			DeviceInfo di;
			memcpy(di.address,devaddr,sizeof(DeviceInfo));
			di.lastTS=0;
			di.lastvalue=DEVICE_DISCONNECTED_C;
			devices.push_back(di);
		}
	};

	void redetectSensors(){
		DS18B20.begin();
		unsigned int devs = DS18B20.getDeviceCount();
		for(int i=0;i<devs; i++){
			DeviceAddress devaddr;
			if( DS18B20.getAddress(devaddr, i) ){// Search the wire for address
				updateDevices(devaddr);
			} else if(!alldisconnected){
				Serial.println(RF("Found ghost DS18B20 device at ")+String(i)+RF(" but could not detect address. Check power and cabling"));
			}

			//Get resolution of DS18b20
			//		Serial.print(RF("Resolution: "));
			//		Serial.println(DS18B20.getResolution( devAddr[i] ));

			//Read temperature from DS18b20
			//		float tempC = DS18B20.getTempC( devAddr[i] );
			//		Serial.println(RF("Temp C: "));
		}
	}

	void SetupDS18B20(){
		DS18B20.begin();

		Serial.print(RF("Parasite power is: "));
		if( DS18B20.isParasitePowerMode() )  Serial.println(RF("ON"));
		else Serial.println(RF("OFF"));

		redetectSensors();
		return;
	}
};

#define FTEMPLATE ".irom.text.dsbreader2"


//------------------------------------------
//Convert device id to String
String GetAddressToString(DeviceAddress deviceAddress){
	String str = "";
	for (uint8_t i = 0; i < 8; i++){
		if( deviceAddress[i] < 16 ) str += String(0, HEX);
		str += String(deviceAddress[i], HEX);
	}
	return str;
}


GenObjMap<unsigned int, DSBBus*> *_buses=0;

DSBBus *getDSBBus(unsigned int pin){
	if(!_buses) _buses=new GenObjMap<unsigned int, DSBBus*>();	// this will never be destroyed, still better than a static instance
	if(_buses->has(pin)) return _buses->get(pin);
	DSBBus* b=new DSBBus(pin);
	_buses->set(pin,b);
	return b;
}

#define DEVICE_INIT 85.0f

bool DSBReader::sensorTick(){	// read temp
//	Serial.println(RF("DSBReader sensorTick"));
	//		if(dsbbus->reading) Serial.println("true");
	long sts=dsbbus->getTimestamp(id);
	if(!dsbbus->reading && (sts-lastts)<dsbbus->timeout) { //if value it has is not recent, and is not reading then tell to read
		dsbbus->read();
		if(dsbbus->reading) return false;
		else {saveValue(tempname,RF(DISCONNECTED_STATE));return true;} // if cannot read -> no sensors connected
	}
//		Serial.println(RF("DSBReader pretick"));
	dsbbus->tick();
//		Serial.println(RF("DSBReader posttick"));
	// get reading
	if(id>=dsbbus->count() || dsbbus->alldisconnected) {
		saveValue(tempname,RF(DISCONNECTED_STATE));
		return true;
	}

// 	Serial.println(String()+RF("DSBReader sts:")+to_string(sts).c_str());
//	Serial.println(String()+RF("DSBReader lasts:")+to_string(lastts).c_str());
	if(sts!=lastts) {
//
		lastts=sts;
		temp=dsbbus->getTemperature(id);
//		Serial.println(String("DSBReader sensorTick temp id:")+to_string(id).c_str()+" val:"+String(temp).c_str());
		if (temp==DEVICE_DISCONNECTED_C){
			saveValue(tempname,RF(DISCONNECTED_STATE));
			return true;
		}
//				Serial.println(String("DSBReader not a disconnect"));
		if(String(temp)==String(DEVICE_INIT)){
			saveValue(tempname,RF(INIT_STATE));
			return true;
		}
//				Serial.println(String("DSBReader not an init"));

		saveValue(tempname,to_stringWithPrecision(temp,2));

		// should emit a disconnect captured at iodevice level
		return true;
	}
	return false;//continue trying
}



bool DSBReader::init() {
	// should check the sensors exist
	println(RF("DSBReader init with success!"));
	tickms=100;
	minafterread=0;
	dsbbus=getDSBBus(pin);
	//lastts=dsbbus->lastTemp;

	StringMapEvent arg2({{tempname+RF("/type"),RF("input")},{tempname+RF("/unit"),RF("°C")}});
	emit(&arg2);

	return true;
};

#endif
