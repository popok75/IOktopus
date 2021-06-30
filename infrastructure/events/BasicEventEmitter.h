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
	bool hasListener(GenString ename);
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

bool BasicEventEmitter::hasListener(GenString ename){return listeners.has(ename);}

bool BasicEventEmitter::emit(GenString ename, Event*event){//sync emit,
	bool b=false;
	std::vector<Subscription> vect= listeners.getAll(ename);
	for(Subscription er:vect){
		if(er.listener){er.listener->notify(ename,event);b=true;}	// TODO : remove once

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

#define ticket_t uint32_t


class AsyncBasicEventEmitter: public BasicEventEmitter{

	struct EventTask {
		GenString key;
		Event *event;
		ticket_t ticketnum;
		EventTask(GenString key0, Event * event0, ticket_t ticketnum0):key(key0),event(event0), ticketnum(ticketnum0){};
		bool equals(GenString key2,Event *event2){return (key==key2 && event==event2);}	// this works only if event=0, or else we need to implement equals() for events
	};
	std::vector<EventTask> eventtasks;
	ticket_t lastEmitTicket=0, lastPropagateTicket=0;;

public:

	ticket_t nextEmitTicket(){lastEmitTicket++;return lastEmitTicket;};

	ticket_t getPropagateTicket(){return lastPropagateTicket;}
	ticket_t getEmitTicket(){return lastEmitTicket;}

	virtual bool emit(GenString key, Event *event=0){	// dont propagate, just push for later


		if(!hasListener(key)) return false; 	// drop if there is no listeners for this key

		ticket_t tic=nextEmitTicket();



//		std::cout << "AsyncBasicEventEmitter::emit path :" << key << " " << tic << std::endl;
		for(unsigned int i=0;i<eventtasks.size();i++){
			if(eventtasks[i].equals(key,event)) {
				eventtasks.erase(eventtasks.begin()+i);
				eventtasks.push_back(EventTask(key,event,tic));	// move the task to the end
				return true;}	// if already in the list, drop it
		}
		if(event) event=event->getCopy();
		eventtasks.push_back(EventTask(key,event,tic));
		//dataeventtimer.once_ms(INTER_EVENT_MS, statictick, this);
		return true;
	};


	virtual bool propagateOnce(){	// return false if there in no more events
		if(eventtasks.empty()) return false;

		EventTask et=eventtasks.front();	// FIFO to preserve order

		eventtasks.erase(eventtasks.begin());

		//bool b=
//		std::cout<< "AsyncBasicEventEmitter:propagateOnce::  key:" << et.key << " "<< et.ticketnum<<", event:" <<et.event <<std::endl;
		lastPropagateTicket=et.ticketnum;

		BasicEventEmitter::emit(et.key,et.event);// Returns true if the event had listeners, false otherwise.
		delete et.event;
		return !eventtasks.empty();
		// if we move to async we should use event instead *event ?
	}
};




#include "../CompatTicker.h"
#define INTER_EVENT_MS 1
class TimerAsyncEmitter: public AsyncBasicEventEmitter{
	Ticker dataeventtimer;
	// separate async emitter part and timer part
	static void statictick(TimerAsyncEmitter *emitter){if(emitter) emitter->tick();}	// dangerous, but safe here our object is not supposed to disappear
	void tick(){
//		bool b;
//		while(b) b=propagateOnce();
		bool b=propagateOnce();

		if(b) {
//			std::cout <<"TimerAsyncEmitter oncems"<<std::endl;
			dataeventtimer.once_ms(INTER_EVENT_MS, statictick, this);
		}
	};
public:
	virtual bool emit(GenString key, Event *event=0){	// dont propagate, just push for later
		AsyncBasicEventEmitter::emit(key,event);
//		std::cout <<"TimerAsyncEmitter oncems 2"<<std::endl;
		dataeventtimer.once_ms(INTER_EVENT_MS, statictick, this);
		return true;
	};
};
#endif
