#ifndef IOLOGGERV020_H
#define IOLOGGERV020_H

#include "../../infrastructure/SyncedClock.h"
#include "../../datastruct/GenString.h"

#include "IOLoggerGen.h"
#undef FTEMPLATE
#define FTEMPLATE ".irom.text.iologgerv20"

#define REFRESHPERIODCONST 10	//second between each reading saved
//#define JSONDQ false
//#define JSONMILLIS false

#define LOGNODESPATH "/nodes/"


#include "IOLoggerFileStore.h"

class IOLoggerv020: public IOLoggerGen
{
	/*
	- getjson from
			- opt1: ignore from, give files list. client can figure what start end when loaded
			- opt2: read files at start, add start-end info to honor from
	- savetolog
			- see if we want to keep data give it to filestore
			- filestore see if we have
					- 1 stream open, if not start one "]]["
					- 1 file open, if not open last or a new file (possibly erase an old one)
					- if last can take X more data if not close the stream open new file

	*/

	IOLoggerFileStore logdata;

public:

	unsigned int refreshPeriodSec;	// storing and using ms when refresh is e.g. 10 seconds is useless

	class LogSyncer : public ClockListener{	// should go to IOLoggerGen
		IOLoggerv020 *mylogger;
	public:
		LogSyncer(IOLoggerv020*mylogger0): mylogger(mylogger0){};
		virtual void clockresync(uint64_t diffms){
			println(RF("Logv016Syncer:")+to_string(diffms));
			if(mylogger) mylogger->resync(diffms);
		};
	};

	public:


	IOLoggerv020 (GenMap *config0): IOLoggerGen(config0){
		refreshPeriodSec=REFRESHPERIODCONST;

		readConfig();
		println(RF("Constructor IOLoggerv020"));
	};
	void resync(uint64_t diffms){logdata.resync(diffms);}


	virtual GenString getJson(unsigned int i=0){return logdata.getFilesJson(i);};
	virtual GenString getJson(){return logdata.getFilesJson(0);};


	void readConfig(){
		if(config) {
			GenString str=config->get(RF(LOGREFRESHPERIOD_KEYWORD));
			if(isDigit(str)) refreshPeriodSec=strToDouble(str);

			//instead should get file ring config (number of files, )

//			str=config->get(RF(LOGMINMEMORYLIMIT_KEYWORD));
//			if(isDigit(str)) logdata.minmemory=strToDouble(str);
//			str=config->get(RF(MAXTIMEPOINTS_KEYWORD));
//			if(isDigit(str)) logdata.sizelimit=strToDouble(str);
		}
	}

	//	virtual GenString getJson(){};
	//	virtual bool notify(GenString ename,Event *e){};

	uint64_t getMillis64(){return CLOCK32.getMS();}//millis64();};



	bool notify(GenString command, Event *event){
		if(command==RF(MODEL_UPDATED_EVENT)){	// use events that has both ts and val
//			println("IOLoggerv020::notify MODELUPDATEDEVENT");
			StringMapEvent *evmap=0;
			if(event->getClassType()==StringMapEventTYPE) evmap=(StringMapEvent*)(event);
			if(!evmap) return false;
			GenString vnamesave;

			// find a val, check if we have the ts, save it
			std::vector<GenString> loaded ;
			// extract first var name
				// see if we have a val and ts -> save
			GenString nodespath=RF(LOGNODESPATH);
			for(auto it : evmap->values){
				GenString key=it.key();
				GenString vname=getPathBranch(key).erase(0,nodespath.size());
				logdata.preloadVarname(vname);
				bool newvar=true;
				for(GenString ls : loaded) if(ls==vname) {newvar=false;break;}
				if(!newvar) continue;
	//			println(GenString()+key);
	//			println(GenString()+vname);
				GenString valstr=evmap->values.get(nodespath+vname+'/'+RF(VALUE_FIELD));
				if(valstr.empty()) continue;
				GenString tsstr=evmap->values.get(nodespath+vname+'/'+RF(TIMESTAMP_FIELD));
				if(isDigit(valstr)) {
					double dval=strToDouble(valstr);
					uint64_t nts=0;
					if(isDigit(tsstr)) nts=strToUint64(tsstr);
					saveToLog(vname,dval,nts);
					loaded.push_back(vname);
				}
			}
			if(!loaded.empty()) return true;
			else return false;

		}

		if(command==RF(GET_JSON_LOG_EVENT)){
			NamedStringMapEvent *strev=0;
			if(event->getClassType()==NamedStringMapEventTYPE) strev=(NamedStringMapEvent*)(event);
			if(!strev) return false;

			unsigned long id=0;
			GenString idstr=strev->values.get(RF("id"));
			if(isDigit(idstr)) id=strToUint64(idstr);

			strev->ename=getJson(id);//logdata.getFilesJson();
			return true;

		}
		return false;
	}






	bool saveToLog(GenString cname, double dval,uint64_t tsdata){
#ifdef ESP8266
 	print(RF("IOLoggerv020::saveToLog - memory :"));	 println(ESP.getFreeHeap(),DEC);
#endif
		uint64_t ts=tsdata;
		if(!ts) ts=getMillis64();
		uint64_t diff=logdata.getLastTSMS(cname);
// 		println(GenString()+RF("IOLoggerMemStore::saveToLog : new ts ")+to_string(ts)+RF(" saved ts:")+to_string(diff));
		if(diff) {
//			 println(GenString()+RF("IOLoggerMemStore::saveToLog : new ts ")+to_string(ts)+RF(" saved ts:")+to_string(diff));
			diff=ts-diff;
			diff=diff/1000;
		     println(GenString()+RF("logdiff:")+to_string(diff)+RF(" ")+to_string(refreshPeriodSec));
			if((diff+1)<refreshPeriodSec) return false; // got a value too soon
		}
		println(GenString()+RF("IOLoggerv20: adding data : ")+cname+ " v:"+to_stringWithPrecision(dval,2)+" ts:"+to_string(ts));
		return logdata.addData(ts,cname,dval);
	}

};


#endif

