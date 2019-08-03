#ifndef GENREADER_H
#define GENREADER_H


#include "IODriver.h"
#include "IOReader.h"

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.genreader"

#ifndef VALUE_FIELD
#define VALUE_FIELD "value"
#endif
#ifndef TIMESTAMP_FIELD
#define TIMESTAMP_FIELD "timestamp"
#endif
#ifndef UNIT_FIELD
#define UNIT_FIELD "unit"
#endif
#ifndef TYPE_FIELD
#define TYPE_FIELD "type"
#endif
#ifndef INPUT_TYPE_VALUE
#define INPUT_TYPE_VALUE "input"
#endif

class BasicReader : public IOReader
{
	IODriver *iodriver=0;
public:
	BasicReader(IODriver* iodriver0):iodriver(iodriver0){
	//	println(GenString()+"BasicReader::constructor "+to_string((uint64_t)iodriver0));
	}
	~BasicReader(){if(iodriver) delete iodriver;}
	bool isNamed(std::string name){
		return false;};

	bool init() {
		println(RF("BasicReader::init ")+to_string((uint64_t)iodriver));
		if(!iodriver || !iodriver->init()) return false;

		tickms=iodriver->tickms();
		minafterread=iodriver->minafterread();
		int s=iodriver->channelNumber();
		for(int i=0;i<s;i++){
			initializeChannel(i);//iodriver->channelType(i),iodriver->channelType(i));
		}

		return true;
	};

	void initializeChannel(unsigned int channel){//	//should be based on type not on name
		GenString channeltype=iodriver->channelType(channel);
		GenString channelname=iodriver->channelName(channel);
		if(channeltype==RF(HUMIDITY_CHANNEL_TYPE)){//		std::multimap<std::string,std::string>valmap={{"Humidity/type","input"},{"Humidity/unit","%"}};//,{"ts",to_string(CLOCK.getTimeMS())}};
			StringMapEvent arg({{channelname+'/'+RF(TYPE_FIELD),RF(INPUT_TYPE_VALUE)},{channelname+'/'+RF(UNIT_FIELD),RF("%")}});
	//		emit(&arg);
		}
		if(channeltype==RF(TEMPERATURE_CHANNEL_TYPE)){//		valmap={{"Temperature/type","input"},{"Temperature/unit","°C"}};//,{"ts",to_string(CLOCK.getTimeMS())}};
			StringMapEvent arg2({{channelname+'/'+RF(TYPE_FIELD),RF(INPUT_TYPE_VALUE)},{channelname+'/'+RF(UNIT_FIELD),RF("°C")}});
	//		emit(&arg2);
		}
	}

	bool sensorTick(){	// read temp, then hum
	//	println(GenString()+"BasicReader::sensorTick"+to_string((uint64_t)iodriver));
#ifdef ESP8266
//	print(RF("BasicReader::sensorTick - memory :"));	 println(ESP.getFreeHeap(),DEC);
#endif

		if(!iodriver) return false;
		if(iodriver->sensorTick()){
			// for each channel, see if connected save the value according to name, if not save disconnected
			int s=iodriver->channelNumber();
			for(int i=0;i<s;i++){
				if(iodriver->isConnected(i)) {
					float f=iodriver->value(i);
					if(isnan(f)) saveValue(iodriver->channelName(i),RF(INIT_STATE),iodriver->timestamp(i));
					else saveValue(iodriver->channelName(i),to_stringWithPrecision(iodriver->value(i),2),iodriver->timestamp(i));
				}
				else saveValue(iodriver->channelName(i),RF(DISCONNECTED_STATE));
			}
			return true;
		};

		return false;
	}




	void saveValue(std::string name,std::string value,uint64_t tsms=0){
		save(tsms);

		StringMapEvent arg({{name+'/'+RF(VALUE_FIELD),value}});
		if(tsms) arg.values.set(name+'/'+RF(TIMESTAMP_FIELD),to_string(tsms));
	#ifdef ESP8266
	#ifdef MEMDEBUG
			print(RF("IOReader::saveValue : free memory :"));	println(ESP.getFreeHeap(),DEC);
	#endif
	#endif
	 	emit(&arg); //use 64bit timestamp here

	}

};
#endif


