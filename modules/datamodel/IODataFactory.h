#ifndef IODATAFACTORY_H
#define IODATAFACTORY_H

#include "../../datastruct/GenString.h"

#include "IODataGen.H"
#include "IODatav01.H"

#include "IODatav03/BasicIODataKit.H"

#define KEEPONLYCURRENTVERSION

#define DEFDATAVERSION "03"

class IODataFactory
{
public:
	static IODataGen* create(GenString versionnumber=""){

		if(versionnumber.empty()) versionnumber=RF(DEFDATAVERSION);
#ifndef DEFDATAVERSION
		if(versionnumber==RF("01")) return new IODatav01();
#endif

		// IODatav03

		return BasicIODataKit::createIODatav03AndCo();

	};
};


#endif
