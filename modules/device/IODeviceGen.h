#ifndef IODEVICEGEN_H
#define IODEVICEGEN_H

#include "../../infrastructure/events/DefaultEventEmitter.h"

class IODeviceGen : public DefaultEventEmitter, public EventListener
{
protected:
	// replace by one map and one string

	unsigned int num;
/*	GenString name, modelname;
	std::vector<unsigned int> pins;
	GenMap subpaths;
	GenString path;*/

public:
//	IODeviceGen(unsigned int num0, GenString name0, GenString modelname0,std::vector<unsigned int> pins0, GenMap subpaths0, GenString path0):path(path0){pins=pins0;modelname=modelname0;name=name0;subpaths=subpaths0;num=num0;}
	virtual bool notify(GenString ename,Event*event=0){return false;}; // update the model from the physical device called from the driver
	virtual SensorReader *getReader(){return 0;}
	virtual void init(){};
};



#endif
