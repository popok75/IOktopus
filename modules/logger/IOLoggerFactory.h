#ifndef LOGGERFACTORY_H
#define LOGGERFACTORY_H

#include "../../datastruct/GenString.h"


#include "IOLoggerGen.h"
#include "IOLoggerv01.h"
#include "IOLoggerv015.h"
#include "IOLoggerv016.h"

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.iologgerfactory"

class IOLoggerFactory
{
public:
	static IOLoggerGen* create(GenString versionnumber, GenMap *config){
		if(versionnumber==RF("v0.1")) return new IOLoggerv01(config);
		if(versionnumber==RF("v0.15")) return new IOLoggerv015(config);
		if(versionnumber==RF("v0.16")) return new IOLoggerv016(config);
		return new IOLoggerv016(config);
//		return 0;
	};
};

#endif
