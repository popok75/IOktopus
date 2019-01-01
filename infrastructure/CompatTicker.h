#ifndef COMPATTICKER_H
#define COMPATTICKER_H


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
