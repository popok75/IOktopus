#ifndef IODEVICEFACTORY_H
#define IODEVICEFACTORY_H

#include "../../infrastructure/CompatIO.h"

#include "IODevicev01.h"

#define CURRENT "v01"

class IODeviceFactory
{
public:/*
	static IODeviceGen* create(unsigned int num, GenString name,  GenString model, std::vector<unsigned int> pins, GenMap subpaths,GenString path, GenString versionnumber=CURRENT){
		return new IODevicev01(num, name, model, pins, subpaths, path);
	};*/

	static IODeviceGen* create(unsigned int num,  GenMap *globalconfig, GenString eventname, EventListener *listener){
		IODeviceGen *dev=new IODevicev01(num, globalconfig);
		dev->on(eventname,listener);
		dev->init();
		return dev;
	};
};


#endif
