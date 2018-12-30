#ifndef BASICEVENTEMITTER_H
#define BASICEVENTEMITTER_H

#include "EventEmitter.h"

#include "../../datastruct/GenObjMap.h"


struct Subscription {
	EventListener* listener=0;
	bool once=false;
	Subscription(EventListener* listener0, bool once0=false):listener(listener0), once(once0){};
	Subscription(const Subscription &sub){listener=sub.listener;once=sub.once;};
	Subscription(Subscription *sub){listener=sub->listener;once=sub->once;};
	Subscription(){};
//	inline bool operator==(const Subscription& sub){ return sub.listener==listener && sub.once==once; };
	inline bool operator==(Subscription sub){ return sub.listener==listener && sub.once==once; };
};



class BasicEventEmitter : public EventEmitter{

	//std::vector<Subscription> listeners;
	GenObjMap<GenString, Subscription> listeners;

public:
	BasicEventEmitter():EventEmitter(){};
	void on(GenString ename, EventListener *er);

	void once(GenString ename, EventListener*);
	void removeListener(GenString ename, EventListener*);

	bool emit(GenString ename, Event*);

	void on(EventListener *er){on("",er);};
	bool emit(Event*e){return emit("",e);};
};


void BasicEventEmitter::on(GenString ename, EventListener *er)
{
	Subscription sub(er);
	listeners.set(ename, sub);
};
void BasicEventEmitter::once(GenString ename, EventListener *er)
{	 Subscription sub(er,true);
	 listeners.set(ename, sub);
};

bool BasicEventEmitter::emit(GenString ename, Event*event){//sync emit,
	bool b=false;
	std::vector<Subscription> vect= listeners.getAll(ename);
	for(Subscription er:vect){
		if(er.listener){er.listener->notify(ename,event);b=true;}	//remove once
	}
	return b;
};

void BasicEventEmitter::removeListener(GenString ename, EventListener *el){
//	unsigned int i=0;

	std::vector<Subscription> vect= listeners.getAll(ename);
	std::vector<Subscription> toerase;
	for(Subscription er:vect){
			if(er.listener==el){toerase.push_back(er);}	//remove once
		}
	for(Subscription er:toerase){
		listeners.erase(ename,er);
	}

/*	for(Subscription er:listeners){
		if(er.eventname==ename && er.listener==el) {listeners.erase(listeners.begin()+i);break;}
		i++;
	}*/
};

#endif
