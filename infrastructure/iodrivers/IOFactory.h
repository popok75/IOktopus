#ifndef IOFACTORY_H
#define IOFACTORY_H

#define AUTOREADER_KEYWORD "autoread"

#include "GenReader.h"


//#include "../../datastruct/GenString.h"
//#include <vector>


#include "FakeDriver.h"

#include "PsychroDriver.h"


#ifdef ESP8266BUILD
#include "../esp/DHTDriver.h"

#include "../esp/DSBDriver.h"
#include "../esp/HTUDriver.h"
#include "../esp/SHTDriver.h"

#endif


//static GenObjMap<unsigned int, SensorReader*> *_sensors;
#define SHT15_KEYWORD "SHT15"
#define HTU21_KEYWORD "HTU21"
#define DHT22_KEYWORD "DHT22"
#define DS18B20_KEYWORD "DS18B20"
#define PSYCHRO_KEYWORD "Psychrometer"

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.iofactory"


SensorDriver *IOFactorycreateDriver (GenString model0,unsigned int num,std::vector<unsigned int> &pinvect){
	SensorDriver *ndriver=0;
#ifdef x86BUILD
	if(model0==RF(SHT15_KEYWORD)) ndriver= new FakeTempRHDriver(num,pinvect);
 	if(model0==RF(HTU21_KEYWORD)) ndriver= new FakeTempRHDriver(num,pinvect);
 	if(model0==RF(DHT22_KEYWORD)) ndriver= new FakeTempRHDriver(num,pinvect);
	if(model0==RF(DS18B20_KEYWORD)) ndriver= new FakeTempRHDriver(num,pinvect);
#endif

#ifdef ESP8266BUILD
	if(model0==RF(SHT15_KEYWORD)) ndriver= new SHTDriver(num,pinvect);
	if(model0==RF(DHT22_KEYWORD)) ndriver= new DHTDriver(num,pinvect);
	if(model0==RF(DS18B20_KEYWORD)) ndriver= new DSBDriver(num,pinvect);
	if(model0==RF(HTU21_KEYWORD)) ndriver= new HTUDriver(num,pinvect);
#endif
	if(model0==RF(PSYCHRO_KEYWORD)) ndriver= new PsychroDriver(num,pinvect);

	if(!ndriver) println(RF("Sensor model unknown : '")+model0+RF("'"));

	return ndriver;
};

class IOFactory {

public:
	static bool isSensor (GenString model0){return true;};
/*
	static SensorReader*getReader(std::vector<unsigned int> pins0){
	//	if(!_sensors) return 0;
	//	for(unsigned int i:pins0) if(_sensors.has(i)) return _sensors.get(i);
		return 0;
	}
*/
//	static SensorReader *createSensor (GenString model0, std::vector<unsigned int> &pins, GenString name=""){
#undef FTEMPLATE
#define FTEMPLATE ".irom.text.iofactory4"
	static SensorReader *createReader (unsigned int num, GenMap *config){ //GenString model0, std::vector<unsigned int> &pins, GenString name=""){
 //		println(RF("IOFactory::createSensor"));

		GenString pins=config->get(GenReader::getFN(RF("pins"),num));
		std::vector<unsigned int> pinvect=getPins(pins);
		GenString model0=config->get(GenReader::getFN(RF("model"),num));
		SensorDriver *ndriver=IOFactorycreateDriver(model0,num,pinvect);
		if(!ndriver) return 0;
		GenReader *genreader=new GenReader(ndriver);

#ifdef ESP8266BUILD
	print(RF("IOFactory::createSensor - memory after end:"));	 println(ESP.getFreeHeap(),DEC);
#endif
		return genreader;
	};
#undef FTEMPLATE
#define FTEMPLATE ".irom.text.iofactory3"
	static unsigned int getPin(GenString pinstr){
		unsigned int p=255;
		while(pinstr[0]==' ') pinstr.erase(0,1);
		while(pinstr[pinstr.length()-1]==' ') pinstr.erase(pinstr.length()-1,1);
		if(pinstr==RF("D0")) p=D0;
		if(pinstr==RF("D1")) p=D1;
		if(pinstr==RF("D2")) p=D2;
		if(pinstr==RF("D3")) p=D3;
		if(pinstr==RF("D4")) p=D4;
		if(pinstr==RF("D5")) p=D5;
		if(pinstr==RF("D6")) p=D6;
		if(pinstr==RF("D7")) p=D7;
		return p;
	}

	static std::vector<unsigned int>getPins(GenString pins){
		std::vector<unsigned int> vect;
		unsigned int ip=0,i=pins.find(",",ip);
		while(i<pins.size()){
			GenString sub=pins.substr(ip,i);
			unsigned int p=getPin(sub);
			if(p!=255) {
				vect.push_back(p);
		//		println(GenString()+RF("getPins(): added pin:")+sub+RF(" ")+to_string(p));
			}
			ip=i+1;
			i=pins.find(",",ip);
		}
		if(ip<pins.size()){
			GenString sub=pins.substr(ip);
			unsigned int p=getPin(sub);
			if(p!=255) {
				vect.push_back(p);
		//		println(GenString()+RF("getPins(): added pin:")+sub+RF(" ")+to_string(p));
			}
		}
		return vect;
	}

};



#endif
