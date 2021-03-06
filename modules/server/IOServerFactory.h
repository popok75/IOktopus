#ifndef IOSERVERFACTORY_H
#define IOSERVERFACTORY_H

#include "../../datastruct/GenString.h"


//#include "IOServerGen.h"
#include "IOServerv01.h"

class IOServerFactory
{
public:
	static IOServerv01* create(GenString name,GenString versionnumber, GenMap *config=0){
		return new IOServerv01(name,config);
	};
};

#endif
