#ifndef EVENT_H
#define EVENT_H

#define EventTYPE 0

class Event{
public:
	virtual ~Event(){};
	virtual Event* getCopy(){return new Event();};
	virtual unsigned int getClassType(){return EventTYPE;}
};

#include "../../datastruct/GenString.h"

#define StringEventTYPE 1
class StringEvent:public Event {
public:
	virtual Event* getCopy(){return new StringEvent(str);};
	GenString str;
	StringEvent(GenString str0=""):str(str0){};
	virtual unsigned int getClassType(){return StringEventTYPE;}
};
#define MultipartStringEventTYPE 4
class MultipartStringEvent : public StringEvent {
public:
	virtual Event* getCopy(){return new MultipartStringEvent(maxsize,index, str);};
	unsigned int index=0;
	unsigned long maxsize=0, totalsize=0;
	bool tobecontinued=false;
	MultipartStringEvent(unsigned int maxsize0, unsigned int index0=0,GenString str0=""):StringEvent(str0){maxsize=maxsize0;};
	virtual unsigned int getClassType(){return MultipartStringEventTYPE;}
};

/*
#define LogRetreiveEventTYPE 5
class LogRetreiveEvent : public MultipartStringEvent {
	virtual Event* getCopy(){return new MultipartStringEvent(maxsize,index, str);};
public:
	uint64_t timestamp=0;
	LogRetreiveEvent(uint64_t ts0, unsigned int maxsize0, unsigned int index0=0,GenString str0=""):MultipartStringEvent(maxsize0,index0,str0), timestamp(ts0){maxsize=maxsize0;};
	virtual unsigned int getClassType(){return LogRetreiveEventTYPE;}
};
*/


class CallbackEvent :public Event{
public:
	virtual Event* getCopy(){return new CallbackEvent();};
	virtual void callback(void*arg){};
};

#include "../../datastruct/GenMap.h"
/*
#define NamedStringMapEventTYPE 3
class NamedStringMapEvent : public Event{
public:
	std::string ename;
	GenMap values;
	NamedStringMapEvent(std::string ename0,GenMap *src) : ename(ename0), values(*src){};
	NamedStringMapEvent(std::string ename0,std::initializer_list<std::initializer_list<GenString>> src): ename(ename0), values(src)  {};
	NamedStringMapEvent(std::string ename0,std::multimap<GenString,GenString> *src): ename(ename0), values(*src)  {};
	virtual Event* getCopy(){return new NamedStringMapEvent(ename,&values);};
	virtual unsigned int getClassType(){return NamedStringMapEventTYPE;}
};
*/
#define StringMapEventTYPE 2
class StringMapEvent : public Event{
public:
	GenMap values;
	StringMapEvent(GenMap *src): values(*src) {}
	StringMapEvent(std::multimap<GenString,GenString> *src): values(*src)  {};
	StringMapEvent(std::initializer_list<std::initializer_list<GenString>> src): values(src)  {};
	virtual Event* getCopy(){return new StringMapEvent(&values);};
	virtual unsigned int getClassType(){return StringMapEventTYPE;}
};

#define NamedStringMapEventTYPE 3
class NamedStringMapEvent : public StringMapEvent{
public:
	std::string ename;
	NamedStringMapEvent(std::string ename0,GenMap *src) : StringMapEvent(src), ename(ename0){};
	NamedStringMapEvent(std::string ename0,std::initializer_list<std::initializer_list<GenString>> src): StringMapEvent(src), ename(ename0) {};
	NamedStringMapEvent(std::string ename0,std::multimap<GenString,GenString> *src):  StringMapEvent(src), ename(ename0) {};
	virtual Event* getCopy(){return new NamedStringMapEvent(ename,&values);};
	virtual unsigned int getClassType(){return NamedStringMapEventTYPE;}
};


#endif
