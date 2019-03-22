#ifndef ERRORLOGFILE_H
#define ERRORLOGFILE_H

#include "CompatFS.h"
#include "../datastruct/GenString.h"
#include "SyncedClock.h"

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.errorlogfile"

#define MAXERRORLOGSIZE 50000

GenString errorLogFilename="/errorlog.txt";	//cannot be RF()

void eraseErrorLogFile(){
	CURFS.erase(errorLogFilename);
};

void appendErrorLogFile(GenString str){
	str=CLOCK32.getNowAsString()+" - "+str+"\n";
	std::vector<unsigned char> vect(str.begin(),str.end());
	if(CURFS.fileSize(errorLogFilename)>=MAXERRORLOGSIZE) eraseErrorLogFile();// autoerase when reach max size
	CURFS.appendToFile(errorLogFilename,vect);
};

void errLog(GenString str){appendErrorLogFile(str);}
void errLog(GenString &str){appendErrorLogFile(str);}
#endif
