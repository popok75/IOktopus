#ifndef IODATAGEN_H
#define IODATAGEN_H

#include "../../datastruct/GenTreeMap.H"
#include "../../datastruct/GenMap.H"

#include "../../infrastructure/events/DefaultEventEmitter.H"

/*
#define VALUEFIELD "val"
#define MINFIELD "minval"
#define MAXFIELD "maxval"
*/
#define VALUEFIELD RF("val")
#define TSFIELD RF("ts")
#define MINFIELD RF("minval")
#define MAXFIELD RF("maxval")
#define MODELUPDATED RF("modelUpdated")
#define GETASJSON RF("getAsJson")
#define	UPDATEMODEL RF("updateModel")
#define	SLASH RF("/")

class IODataGen : public EventListener, public DefaultEventEmitter
{
protected:
	GenTreeMap data;	// devices should be addressed at /devices/ as if part of the data
public:
	virtual ~IODataGen(){};

 	virtual bool update(GenString path, GenMap map){return false;};	//can we use reference instead without ambiguous
 	bool updateVal(GenString path, GenString val){return false;}
 	virtual GenString get(GenString path){return GenString();}

 	virtual GenString getAsJson(){return data.getAsJson();}

 	virtual bool notify(GenString ename,Event*event=0){return false;};

 };

#endif
