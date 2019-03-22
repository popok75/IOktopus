#ifndef LOGGERFACTORY_H
#define LOGGERFACTORY_H

#include "../../datastruct/GenString.h"

#define DEFLOGGERVERSION "v0.16"

#include "IOLoggerGen.h"
//#include "IOLoggerv01.h"	// no more compatible
//#include "IOLoggerv015.h"
#include "IOLoggerv016.h"
#include "IOLoggerv020.h"

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.iologgerfactory"

#define LOGGERTYPE "log-type"

class IOLoggerFactory
{
public:
	static IOLoggerGen* create(GenMap *config, GenString versionnumber=""){
		if(config) {
			if(config->get(RF(LOGGERTYPE))=="memory") versionnumber=RF("v0.16");
			if(config->get(RF(LOGGERTYPE))=="file") versionnumber=RF("v0.20");
		}
		if(versionnumber=="") versionnumber=RF(DEFLOGGERVERSION);
//		if(versionnumber==RF("v0.1")) return new IOLoggerv01(config);
//		if(versionnumber==RF("v0.15")) return new IOLoggerv015(config);
		if(versionnumber==RF("v0.16")) return new IOLoggerv016(config);
		if(versionnumber==RF("v0.20")) return new IOLoggerv020(config);
		return new IOLoggerv020(config);
//		return 0;
	};
};

#endif
