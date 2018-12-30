#ifndef IOLOGGERV016_H
#define IOLOGGERV016_H

#include "../../infrastructure/SyncedClock.h"
#include "../../datastruct/GenString.h"

#include "IOLoggerGen.h"
#undef FTEMPLATE
#define FTEMPLATE ".irom.text.iologgerv16"

#define REFRESHPERIODCONST 10	//second between each reading saved
#define JSONDQ false
#define JSONMILLIS false


#include "IOLoggerMemStore.h"

class IOLoggerv016: public IOLoggerGen
{
	IOLoggerMemStore logdata;

	TimeStampType savedfirstts=0;
	unsigned int savedindex=0;
	bool savedindexset=false;

	//	unsigned int maxsize=6*24*3;


	unsigned int refreshPeriodSec;	// storing and using ms when refresh is e.g. 10 seconds is useless

	class Logv016Syncer : public ClockListener{	// should go to IOLoggerGen
		IOLoggerv016 *mylogger;
	public:
		Logv016Syncer(IOLoggerv016*mylogger0): mylogger(mylogger0){};
		virtual void clockresync(uint64_t diffms){
			println(RF("Logv016Syncer:")+to_string(diffms));
			if(mylogger) mylogger->resync(diffms);
		};
	};

	public:

	void resync(uint64_t diffms){

		logdata.resync(diffms);
		//	for(std::vector<StreamCell>::iterator it=logdata.begin();it!=logdata.end();it++) it->resync(diffms);	// correct each timestamp
	}

	IOLoggerv016 (GenMap *config0): IOLoggerGen(config0){
		refreshPeriodSec=REFRESHPERIODCONST;

		readConfig();
		println(RF("Constructor IOLoggerv016"));
	};

	void readConfig(){
		if(config) {
			GenString str=config->get(RF(LOGREFRESHPERIOD_KEYWORD));
			if(isDigit(str)) refreshPeriodSec=strToDouble(str);
			str=config->get(RF(LOGMINMEMORYLIMIT_KEYWORD));
			if(isDigit(str)) logdata.minmemory=strToDouble(str);
			str=config->get(RF(MAXTIMEPOINTS_KEYWORD));
			if(isDigit(str)) logdata.sizelimit=strToDouble(str);
		}
	}

	//	virtual GenString getJson(){};
	//	virtual bool notify(GenString ename,Event *e){};

	uint64_t getMillis64(){return CLOCK32.getMS();}//millis64();};

	bool notify(GenString command,Event *event){
		if(command==RF("modelUpdated")){
			NamedStringMapEvent *namev=0;
			if(event->getClassType()==NamedStringMapEventTYPE) namev=(NamedStringMapEvent*)(event);
			if(!namev) return false;

			if(startsWith(namev->ename,RF("/nodes/")) && namev->values.has(RF("val"))){	//log only nodes
				GenString strval=namev->values.get(RF("val"));
				if(isDigit(strval)){
					double dval=strToDouble(strval);
					saveToLog(getPathLeaf(namev->ename),dval);
				} else return false;
			}
		}
		if(command==RF("getLogJson")){
			MultipartStringEvent *strev=0;
			if(event->getClassType()==MultipartStringEventTYPE) strev=(MultipartStringEvent*)(event);

			if(!strev) return false;
			if(strev->index==0 && logdata.isLocked()) return false;	// cant make a new during lock
			strev->str.erase();
			if(!MULTIPARTLOG){
				this->writeJson(strev->str);
			} else {
				logdata.setLocked(true);
				strev->tobecontinued=this->writeJson(strev->str,strev->maxsize);
				logdata.setLocked(strev->tobecontinued);
				if(!strev->tobecontinued) println(GenString()+RF("IOLoggerv016::notify post : total dots :")+to_string(logdata.size()));
			}
		//	println(GenString()+RF("IOLoggerv016::notify json:")+(strev->str));
#ifdef ESP8266
			//				print("IOLoggerv016::notify post : free memory :");	println(ESP.getFreeHeap(),DEC);
#endif
			return true;
		}
		return false;
	}



	bool saveToLog(GenString cname, double dval){
#ifdef ESP8266
 	print(RF("IOLoggerv016::saveToLog - memory :"));	 println(ESP.getFreeHeap(),DEC);
#endif
		uint64_t ts=getMillis64();
		uint64_t diff=logdata.getLastTSMS(cname);
//		println(GenString()+RF("IOLoggerMemStore::saveToLog : new ts ")+to_string(ts)+RF(" saved ts:")+to_string(diff));
		if(diff) {
			//		println(GenString()+RF("IOLoggerMemStore::saveToLog : new ts ")+to_string(ts)+RF(" saved ts:")+to_string(diff));
			diff=ts-diff;
			diff=diff/1000;
			//	    println(GenString()+RF("logdiff:")+to_string(diff)+RF(" ")+to_string(refreshPeriodSec));
			if(diff<refreshPeriodSec) return false; // got a value too soon
		}
//		println("adding data");
		return logdata.addData(cname,dval,ts);
	}




	virtual GenString getJson(){
		GenString json;
		writeJson(json);
		return json;
	}

	bool writeJson(GenString &json2, unsigned int maxsize=0){	//return true to be called for more return false when finished

//		println(GenString()+RF("IOLoggerv016::writeJson logdata.size():")+to_string(logdata.size()));
		bool limit=true;
		if(maxsize==0) limit=false; //no limit

		if(logdata.size()==0) {json2= RF("[]");return false;}
		GenString json;

		if(!savedindexset){
			json+=RF("[[");
			bool first=true;
			auto names=logdata.getColumnNames();
			for(unsigned int i=0;i<names.size();i++){
				GenString name=names[i];
				if(!first) json+=RF(",");
				first=false;
				json+=RF("\"")+name+RF("\"");
			}
			json+=RF("]");
		}

		unsigned int dots=logdata.size();
//		println(GenString()+RF("IOLoggerv016::writeJson dots:")+to_string(logdata.size()));
		for(unsigned int i=savedindex;i<dots;i++){
//			println(GenString()+RF("IOLoggerv016::writeJson loop json:")+json+" json2:"+json2);
			json+=RF(",[");
			IOLoggerMemStore::FullCell fc=logdata.getIndex(i);
			uint64_t ts=fc.ts;
			if(!savedfirstts) savedfirstts=ts; else ts-=savedfirstts;	//
			json+=to_string(ts);
			for(unsigned int j=0;j<fc.cells.size();j++){
				unsigned int col=fc.cells[j].column;
				json+=GenString()+RF(",")+to_string(col)+RF(",");
				json+=to_stringWithPrecision(fc.cells[j].val,2);
			}
			json+=RF("]");
			if(!limit || (json.size()+json2.size())<maxsize) {json2+=json;json.erase();}
			else {savedindexset=true;savedindex=i;return true;}
		}
//		println(GenString()+RF("IOLoggerv016::writeJson postloop json:")+json+" json2:"+json2);
		json+=RF("]");
		json2+=json;json.erase();
		savedindex=0;
		savedindexset=false;
		savedfirstts=0;
//		println(GenString()+RF("IOLoggerv016::writeJson end json:")+json+" json2:"+json2);
		return false;
	}


};


#endif

