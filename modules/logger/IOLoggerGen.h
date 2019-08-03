#ifndef IOLOGGERGEN_H
#define IOLOGGERGEN_H

#include "../../infrastructure/CompatPrint.h"
//#include "../datamodel/IODataGen.h"   			// no need
//#include "../../infrastructure/CompatNet.h"		// what for

#include "../moduleshared.h"


#undef FTEMPLATE
#define FTEMPLATE ".irom.text.iologgergen"

#define LOGREFRESHPERIOD_KEYWORD "log-refreshperiodsec"
#define LOGMINMEMORYLIMIT_KEYWORD "log-minmemorylimit"
#define MAXTIMEPOINTS_KEYWORD "log-maxtimepoints"



class IOLoggerGen : public EventListener
{
protected:
	GenMap *config;
public:

	IOLoggerGen (GenMap *config0):config(config0){println(RF("Constructor IOLoggerGen"));};

	virtual ~IOLoggerGen(){};

	virtual GenString getJson()=0;

	virtual bool notify(GenString ename,Event *e)=0;//{return false;};
};



#endif
