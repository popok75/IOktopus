#ifndef GENREADER_H
#define GENREADER_H


#include "../iodrivers/SensorReader.h"

#include "SensorDriver.h"

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.genreader"


class GenReader : public SensorReader
{
	SensorDriver *sensordriver=0;
public:
	GenReader(SensorDriver* sensordriver0):sensordriver(sensordriver0){
	//	println(GenString()+"GenReader::constructor "+to_string((uint64_t)sensordriver0));
	}
	~GenReader(){if(sensordriver) delete sensordriver;}
	bool isNamed(std::string name){
		return false;};

	bool init() {
		println(RF("GenReader::init ")+to_string((uint64_t)sensordriver));
		if(!sensordriver || !sensordriver->init()) return false;

		tickms=sensordriver->tickms();
		minafterread=sensordriver->minafterread();
		int s=sensordriver->channelNumber();
		for(int i=0;i<s;i++){
			initializeChannel(i);//sensordriver->channelType(i),sensordriver->channelType(i));
		}

		return true;
	};

	void initializeChannel(unsigned int channel){//	//should be based on type not on name
		GenString channeltype=sensordriver->channelType(channel);
		GenString channelname=sensordriver->channelName(channel);
		if(channeltype==RF(HUMIDITY_CHANNEL_TYPE)){//		std::multimap<std::string,std::string>valmap={{"Humidity/type","input"},{"Humidity/unit","%"}};//,{"ts",to_string(CLOCK.getTimeMS())}};
			StringMapEvent arg({{channelname+RF("/type"),RF("input")},{channelname+RF("/unit"),RF("%")}});
			emit(&arg);
		}
		if(channeltype==RF(TEMPERATURE_CHANNEL_TYPE)){//		valmap={{"Temperature/type","input"},{"Temperature/unit","°C"}};//,{"ts",to_string(CLOCK.getTimeMS())}};
			StringMapEvent arg2({{channelname+RF("/type"),RF("input")},{channelname+RF("/unit"),RF("°C")}});
			emit(&arg2);
		}
	}

	bool sensorTick(){	// read temp, then hum
	//	println(GenString()+"GenReader::sensorTick"+to_string((uint64_t)sensordriver));
#ifdef ESP8266
//	print(RF("GenReader::sensorTick - memory :"));	 println(ESP.getFreeHeap(),DEC);
#endif

		if(!sensordriver) return false;
		if(sensordriver->sensorTick()){
	//		println("GenReader::sensorTick 2");
			// for each channel, see if connected save the value according to name, if not save disconnected
			int s=sensordriver->channelNumber();
			for(int i=0;i<s;i++){
	//			println("GenReader::sensorTick 3");
				if(sensordriver->isConnected(i)) {
	//				println("GenReader::sensorTick 4");

					float f=sensordriver->value(i);
					if(isnan(f)) saveValue(sensordriver->channelName(i),RF(INIT_STATE),sensordriver->timestamp(i));
					else saveValue(sensordriver->channelName(i),to_stringWithPrecision(sensordriver->value(i),2),sensordriver->timestamp(i));
				}
				else saveValue(sensordriver->channelName(i),RF(DISCONNECTED_STATE));
			}
			return true;
		};
	//	println("GenReader::sensorTick end false");
		return false;

	}

	static GenString getFN(GenString str,unsigned int nums){
		return RF("device")+to_string(nums)+RF("-")+str;
	}
};
#endif


