/*
  This in an Arduino library to save exception details
  and stack trace to flash in case of ESP8266 crash.
  Please check repository below for details

  Repository: https://github.com/krzychb/EspSaveCrash
  File: EspSaveCrash.h
  Revision: 1.0.2
  Date: 18-Aug-2016
  Author: krzychb at gazeta.pl

  Copyright (c) 2016 Krzysztof Budzynski. All rights reserved.

  This application is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This application is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/
/* modified by me 28/18/2018	*/

#ifndef _ESPSAVECRASHSPIFFS_H_
#define _ESPSAVECRASHSPIFFS_H_

#include "Arduino.h"
//#include "EEPROM.h"
#include "user_interface.h"


/**
 * User configuration of EEPROM layout
 *
 * Note that for using EEPROM we are also reserving a RAM buffer
 * The buffer size will be bigger by SAVE_CRASH_EEPROM_OFFSET than what we actually need
 * The space that we really need is defined by SAVE_CRASH_SPACE_SIZE
 */


/**
 * Structure of the single crash data set
 *
 *  1. Crash time
 *  2. Restart reason
 *  3. Exception cause
 *  4. epc1
 *  5. epc2
 *  6. epc3
 *  7. excvaddr
 *  8. depc
 *  9. adress of stack start
 * 10. adress of stack end
 * 11. stack trace bytes
 *     ...
 */

class EspSaveCrashSpiffs
{
  public:
	EspSaveCrashSpiffs(char *otherfilename=0);
  //  void print(Print& outDevice = Serial);
    void clearFile();
 //   int count();

  private:
    // none
};

void savetoSpiffs(struct rst_info * rst_info, uint32_t stack, uint32_t stack_end,Print& outputDev );

extern EspSaveCrashSpiffs SaveCrashSpiffs;


#define CRASHFILEPATH "/crashdump.txt"
#define MAXCRASHFILESIZE 10000
#endif
