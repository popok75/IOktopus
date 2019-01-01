#ifndef SENSORDRIVER_H
#define SENSORDRIVER_H

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.sensordriver"

#define HUMIDITY_CHANNEL_TYPE "Humidity"
#define TEMPERATURE_CHANNEL_TYPE "Temperature"


class SensorDriver
{
protected:
	unsigned int num;
	std::vector<unsigned int> pins;

public:
	SensorDriver(unsigned int num0, std::vector<unsigned int> pins0):num(num0),pins(pins0){}
	virtual ~SensorDriver(){};

	virtual bool init() {return true;};

 	virtual bool sensorTick()=0;
 	virtual unsigned int channelNumber()=0;
 	virtual GenString channelName(unsigned int i){return channelType(i);};
 	virtual GenString channelType(unsigned int i)=0;
 	virtual bool isConnected(unsigned int i)=0;
 	virtual float value(unsigned int i)=0;
 	virtual uint64_t timestamp(unsigned int i)=0;

 	virtual float tickms(){return 100;};
 	virtual float minafterread(){return 0;};

};

class TempDriver : public SensorDriver {
protected:
	float temp;
	bool disconnected=true;
public:
	TempDriver(unsigned int num0, std::vector<unsigned int> pins0):SensorDriver(num0,pins0){}
	virtual unsigned int channelNumber(){return 1;};
	virtual GenString channelType(unsigned int i){if(i==0) return RF(TEMPERATURE_CHANNEL_TYPE); return "";};

	virtual bool isConnected(unsigned int i){return !disconnected;}
	virtual float value(unsigned int i){if(i==0) return temp;else return NAN;};
	virtual uint64_t timestamp(unsigned int i){return 0;};
};

class TempRHDriver : public SensorDriver {
protected:
	float temp, hum;
	bool disconnected=true;
public:
	TempRHDriver(unsigned int num0, std::vector<unsigned int> pins0):SensorDriver(num0,pins0){}
	virtual unsigned int channelNumber(){return 2;};
	virtual GenString channelType(unsigned int i){if(i==0) return RF("Temperature"); else if(i==1) return RF("Humidity");return "";};
	virtual bool isConnected(unsigned int i){return !disconnected;}
	virtual float value(unsigned int i){if(i==0) return temp; else if(i==1) return hum; return NAN;};
	virtual uint64_t timestamp(unsigned int i){return 0;};
};



#endif


