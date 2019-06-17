#ifndef COMPATFS_H
#define COMPATFS_H

#ifdef x86BUILD
#include "x86/CurFSx86.h"
CurFS CURFS;
#endif


#ifdef ESP8266BUILD
#include "CompatPrint.h"
#include "esp/CurFSEsp8266.h"
#endif



#endif
