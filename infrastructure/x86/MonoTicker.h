#ifndef TICKER_H
#define TICKER_H

#include "monoTimer.h"

class Ticker {
public:
	TaskStruct *ts=0;
	template <typename... PTYPE>
	void once_ms(unsigned int ms, void (func)(PTYPE...), PTYPE... parames){
		if(ts) monoCancelTimeOut(ts);	// cancel previous timeout
		ts=monoSetTimeOut<PTYPE...>(ms,false,func, std::forward<PTYPE>(parames)...);
	};

	template <typename... PTYPE>
	void attach_ms(unsigned int ms, void (func)(PTYPE...), PTYPE... parames){
	//	std::cout << "Ticker::attach_ms *ts:"<<to_string((uint64_t)ts) <<std::endl;
//		if(ts) std::cout << "Ticker::attach_ms id: "<<to_string(ts->id) <<std::endl;
		if(ts) monoCancelTimeOut(ts);	// cancel previous timeout
		ts=monoSetTimeOut<PTYPE...>(ms,true,func, std::forward<PTYPE>(parames)...);
	}

	void detach(){
		if(ts) {monoCancelTimeOut(ts);ts=0;}
	}
};



#endif
