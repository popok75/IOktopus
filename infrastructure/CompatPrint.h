#ifndef COMPATPRINT_H
#define COMPATPRINT_H


#ifdef x86BUILD
#include "x86/printx86.h"
#endif

#ifdef ESP8266BUILD
#include "esp/printESP8266.h"
#endif

#endif
