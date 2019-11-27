#ifndef TICKER_H
#define TICKER_H

#include "monoTimer.h"

class Ticker {
public:
	//	TaskStruct *ts=0;
	uint64_t tsid=0;
	template <typename... PTYPE>
	void once_ms(unsigned int ms, void (func)(PTYPE...), PTYPE... parames){
		detach();
		TaskStruct *ts=monoSetTimeOut<PTYPE...>(ms,false,func, std::forward<PTYPE>(parames)...);
		tsid=ts->id;
	//	std::cout << "Ticker::once_ms attached task id:"<<ts->id<<",debug:"<< debug <<std::endl;
	};

	template <typename... PTYPE>
	void attach_ms(unsigned int ms, void (func)(PTYPE...), PTYPE... parames){
		//	std::cout << "Ticker::attach_ms *ts:"<<to_string((uint64_t)ts) <<std::endl;
		//		if(ts) std::cout << "Ticker::attach_ms id: "<<to_string(ts->id) <<std::endl;
		detach();
		TaskStruct *ts=monoSetTimeOut<PTYPE...>(ms,true,func, std::forward<PTYPE>(parames)...);
		tsid=ts->id;

//	std::cout << "Ticker::attach_ms attached task id:"<<ts->id<<",debug:"<< debug <<std::endl;
	}

	void detach(){
		if(tsid) {
			TaskStruct *pts=monoFind(tsid);
			if(pts) monoCancelTimeOut(pts);
			tsid=0;
		}


	//	std::cout << "MonoTicker::detached"<<std::endl;
	}

};



#endif
