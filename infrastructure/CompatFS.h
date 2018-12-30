#ifndef COMPATFS_H
#define COMPATFS_H

#ifdef x86BUILD
#include "x86/CurFSx86.h"
CurFS CURFS;
#endif

#ifdef ES8266BUILD
#include "esp/CurFSEsp8266.h"

#endif



#endif
