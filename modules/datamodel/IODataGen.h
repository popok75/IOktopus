#ifndef IODATAGEN_H
#define IODATAGEN_H

#include "../../datastruct/GenTreeMap.H"
#include "../../datastruct/GenMap.H"

#include "../../infrastructure/events/DefaultEventEmitter.H"


#include "../moduleshared.h"

#define	SLASH RF("/")



class IODataGen : public EventListener, public TimerAsyncEmitter
{
protected:
	GenTreeMap data;	// devices should be addressed at /devices/ as if part of the data
public:
	virtual ~IODataGen(){};

 	virtual bool update(GenString path, GenMap map)=0;	//can we use reference instead without ambiguous
 	virtual bool updateVal(GenString path, GenString val, bool notifychange=true)=0;
 	virtual GenString get(GenString path)=0;

 	virtual GenString getAsJson()=0;

 	virtual bool notify(GenString ename,Event*event=0)=0;

 };

class IODataGenDummy : public IODataGen
{
protected:
	GenTreeMap data;	// devices should be addressed at /devices/ as if part of the data
public:

 	virtual bool update(GenString path, GenMap map){return false;};	//can we use reference instead without ambiguous
 	bool updateVal(GenString path, GenString val){return false;}
 	virtual GenString get(GenString path){return GenString();}

 	virtual GenString getAsJson(){return data.getAsJson();}

 	virtual bool notify(GenString ename,Event*event=0){return false;};

 };

#endif
