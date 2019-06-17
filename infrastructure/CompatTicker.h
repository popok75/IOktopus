#ifndef COMPATTICKER_H
#define COMPATTICKER_H

bool durationFormat(std::string duration){	//format is dd:hh:mm:ss
	std::vector<std::string> v{explode(duration, ':')};
	if(v.size()>=0 && v.size()<=3) return true;
	else return false;
}

uint64_t durationToMS(std::string duration){	//format is dd:hh:mm:ss
	std::vector<std::string> v{explode(duration, ':')};
	std::vector<int> factor={1000,60,60,24};
	uint64_t fduration=0,f=1;
	for(int i=v.size();i>0;i--){
		f*=factor[v.size()-i];
		fduration+=std::stoull(v[i-1])*f;
	}
	return fduration;
};

#ifdef x86BUILD

#include "x86/MonoTicker.h"
// lowlevel only, use CLOCK32.getMS(); instead
static inline uint64_t millis64(){	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();};

#endif


#ifdef ESP8266BUILD

#include <Ticker.h>
// lowlevel only, use CLOCK32.getMS(); instead
static inline uint64_t millis64(){return millis();};

#endif


#endif
