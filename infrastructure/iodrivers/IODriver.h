#ifndef SENSORDRIVER_H
#define SENSORDRIVER_H


/*
 *	IODriver is a base interface for sensor drivers it imposes implementation of functions for
 *		- metadata : channels, names, etc
 *		- sensortick
 *		- value & ts retrieval
 *
 * */


#undef FTEMPLATE
#define FTEMPLATE ".irom.text.sensordriver"

#define HUMIDITY_CHANNEL_TYPE "Humidity"
#define TEMPERATURE_CHANNEL_TYPE "Temperature"


class IODriver
{
protected:
	unsigned int num;
	std::vector<unsigned int> pins;

public:
	IODriver(unsigned int num0, std::vector<unsigned int> pins0):num(num0),pins(pins0){}
	virtual ~IODriver(){};

	bool debug=false;

	virtual bool init() {return true;};

 	virtual bool sensorTick()=0;
 	virtual unsigned int channelNumber()=0;
 	virtual GenString channelName(unsigned int i){return channelType(i);};
 	virtual GenString channelType(unsigned int i)=0;
 	virtual bool isConnected(unsigned int i)=0;
 	virtual float value(unsigned int i)=0;			// outputs return their state
 	virtual uint64_t timestamp(unsigned int i)=0;

 	virtual float tickms(){return 100;};
 	virtual float minafterread(){return 0;};

 	virtual bool setValue(unsigned int i, float v){return false;};		// used for outputs and eventually sensors calibration //if channel is not an output, v will be discarded
};

class TempDriver : public IODriver {
protected:
	float temp;
	bool disconnected=true;
public:
	bool debug=false;

	TempDriver(unsigned int num0, std::vector<unsigned int> pins0):IODriver(num0,pins0){}
	virtual unsigned int channelNumber(){return 1;};
	virtual GenString channelType(unsigned int i){if(i==0) return RF(TEMPERATURE_CHANNEL_TYPE); return "";};

	virtual bool isConnected(unsigned int i){return !disconnected;}
	virtual float value(unsigned int i){if(i==0) return temp;else return NAN;};
	virtual uint64_t timestamp(unsigned int i){return 0;};
};

class TempRHDriver : public IODriver {
protected:
	float temp, hum;
	bool disconnected=true;
public:
	TempRHDriver(unsigned int num0, std::vector<unsigned int> pins0):IODriver(num0,pins0){}
	virtual unsigned int channelNumber(){return 2;};
	virtual GenString channelType(unsigned int i){if(i==0) return RF("Temperature"); else if(i==1) return RF("Humidity");return "";};
	virtual bool isConnected(unsigned int i){return !disconnected;}
	virtual float value(unsigned int i){if(i==0) return temp; else if(i==1) return hum; return NAN;};
	virtual uint64_t timestamp(unsigned int i){return 0;};
};



#endif


