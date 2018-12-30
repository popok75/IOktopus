#ifndef COMPATCRASH_H
#define COMPATCRASH_H

#ifdef x86BUILD

#define CRASHFILEPATH "crashdump.txt"
class CurCrashSaverx86{//dummy x86BUILD
public:
	CurCrashSaverx86(char *filename=0){};
	void clear(){println(RF("CrashSaverx86 clear"));};
 	std::string print(){println(RF("CrashSaverx86 print")); return RF("no crashes saved yet");};
//	int count(){return 0;};
} CurSaveCrash;
#endif


#ifdef ESP8266BUILD

#define ESPSAVECRASH_LIB
#ifdef ESPSAVECRASH_LIB
#include <EspSaveCrashSpiffs.h>
#endif


class CurCrashSaverESP8266{
	public:
	void clear(){
#ifdef ESPSAVECRASH_LIB
		SaveCrashSpiffs.clearFile();
#endif
	};
	std::string print(){
		std::string str;
#ifdef ESPSAVECRASH_LIB
	//	Serial.println("CurCrashSaverESP8266::print");
		size_t s=0;
		str=CURFS.readFileToString(CRASHFILEPATH,s);
#endif
		return str;
	};

} CurSaveCrash;



/*
 * class StringPrinter : public Print {
public:
  std::string str;
  StringPrinter(){};
  virtual size_t write(const uint8_t character){str+=character;};
  virtual size_t write(const uint8_t *buffer, size_t size){
      str+=std::string((const char *)buffer, size);
    };
};
class CurCrashSaverESP8266{
	public:
	void clear(){
#ifdef ESPSAVECRASH_LIB
		SaveCrash.clear();
#endif
	};
	std::string print(){
		StringPrinter printer;
#ifdef ESPSAVECRASH_LIB
	//	Serial.println("CurCrashSaverESP8266::print");
		SaveCrash.print(printer);
#endif
		return printer.str;
	};
	int count(){
#ifdef ESPSAVECRASH_LIB
		return SaveCrash.count();
#endif
	};
} CurSaveCrash;
*/

#endif




#endif
