#include "../infrastructure/esp/DSBReader.h"



DSBReader *reader,*reader2;

class DeviceListener : public EventListener
{
	bool notify(GenString ename,Event*event=0){
		StringMapEvent *emap=0;
		if(event->getClassType()==StringMapEventTYPE) emap=(StringMapEvent*)(event);
		if(!emap || emap->values.empty()) return false; //pb
		println(GenString()+RF("DeviceListener::notify ")+emap->values.asJson());
	}
} devicelistener;

class DeviceListener2 : public EventListener
{
	bool notify(GenString ename,Event*event=0){
		StringMapEvent *emap=0;
		if(event->getClassType()==StringMapEventTYPE) emap=(StringMapEvent*)(event);
		if(!emap || emap->values.empty()) return false; //pb
		println(GenString()+RF("DeviceListener2::notify ")+emap->values.asJson());
	}
} devicelistener2;

bool testDSB(){

	reader=new DSBReader(D7);
	reader->on(&devicelistener);
	reader->init();
	reader->autoread(5);

	reader2=new DSBReader(D7);
	reader2->on(&devicelistener2);
	reader2->init();
	reader2->autoread(5);

	return true;
};



