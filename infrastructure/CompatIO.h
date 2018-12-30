#ifndef COMPATIO_H
#define COMPATIO_H


#ifdef x86BUILD
#include "x86/MonoTicker.h"

#define D7 7
#define D6 5
#define D5 5
#define D4 4
#define D3 3
#define D2 2
#define D1 1
#define D0 0
#endif

#ifdef ESP8266BUILD
#include <Ticker.h>
#endif


#include "iodrivers/IOFactory.h"


#endif
