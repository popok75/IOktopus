#ifndef IODATAFACTORY_H
#define IODATAFACTORY_H

#include "../../datastruct/GenString.h"

#include "IODataGen.H"
#include "IODatav01.H"

class IODataFactory
{
public:
	static IODataGen* create(GenString versionnumber){
 /*IODataGen* iod;
	iod=new IODatav01();
	return iod;
//	*/	return new IODatav01();
	};
};


#endif
