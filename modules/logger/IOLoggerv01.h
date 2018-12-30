#ifndef IOLOGGERV01_H
#define IOLOGGERV01_H

//#include "../../infrastructure/CompatNet.h"
//#include "../../infrastructure/CompatTicker.h"

//#include "../../infrastructure/SyncedClock.h"
#include "../../datastruct/GenString.h"

#include "IOLoggerGen.h"

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.iologgerv01"

#define REFRESHPERIODCONST 10	//second between each reading saved
#define JSONDQ false
#define JSONMILLIS false

//#define TS64
#define TS32

class IOLoggerv01: public IOLoggerGen
{
	// save the value only every some period drop them otherwise
	// another step is to ask to refresh the sensor (with a timer)

	struct LogValue{
#ifdef TS64
		uint64_t timestamp;		//in ms
#endif
#ifdef TS32
		uint32_t timestamp;		//in s
#endif
		float value;
#ifdef TS64
		LogValue(uint64_t ts0,double val0):timestamp(ts0),value(val0){};
		uint64_t getMS(){return timestamp;};
		uint32_t getS(){return timestamp/1000;};
		void resync(uint64_t diffms){timestamp+=diffms;}
#endif
#ifdef TS32
		LogValue(uint64_t ts0,double val0):timestamp(ts0/1000),value(val0){};
		uint64_t getMS(){return timestamp*1000;};
		uint32_t getS(){return timestamp;};
		void resync(uint64_t diffms){timestamp+=diffms/1000;}
#endif

	};
	struct Channel{
		uint64_t lasttimestamp=0;
		std::vector<LogValue> log;
	};
	GenObjMap<GenString, Channel*> logs;
	unsigned int maxsize=6*24*3;
	unsigned int refreshPeriodSec;	// storing and using ms when refresh is e.g. 10 seconds is useless

	class LogSyncer : public ClockListener{
		IOLoggerv01 *mylogger;
	public:
		LogSyncer(IOLoggerv01*mylogger0): mylogger(mylogger0){};
		virtual void clockresync(uint64_t diffms){
			println(RF("LogSyncer:")+to_string(diffms));
			if(mylogger) mylogger->resync(diffms);
		};
	};

	public:

	void resync(uint64_t diffms){
		for(auto it:logs){
			Channel *c=it.valueRef();
			for(std::vector<LogValue>::iterator jt=c->log.begin();jt!=c->log.end();jt++){
				jt->resync(diffms);	// correct each timestamp
			}
		}
	}

	IOLoggerv01 (GenMap *config0): IOLoggerGen(config0){
		refreshPeriodSec=REFRESHPERIODCONST;
		if(config0) {
			GenString str=config0->get(RF(LOGREFRESHPERIOD_KEYWORD));
			if(isDigit(str)) refreshPeriodSec=strToDouble(str);
		}
		println(RF("Constructor IOLoggerv01")); };

	uint64_t getMillis64(){return CLOCK32.getMS();}//millis64();};

	bool notify(GenString command,Event *event){
		if(command==RF("modelUpdated")){
			NamedStringMapEvent *namev=0;
			if(event->getClassType()==NamedStringMapEventTYPE) namev=(NamedStringMapEvent*)(event);
			if(!namev) return false;

			if(startsWith(namev->ename,RF("/nodes/")) && namev->values.has(RF("val"))){	//log only nodes
				GenString strval=namev->values.get(RF("val"));
				double dval=strToDouble(strval);
				saveToLog(getPathLeaf(namev->ename),dval);
			}
		}
		if(command==RF("getLogJson")){
			MultipartStringEvent *strev=0;
			if(event->getClassType()==MultipartStringEventTYPE) strev=(MultipartStringEvent*)(event);

			if(!strev) return false;

			strev->str.erase();
			uint32_t ts0=millis();
			if(!MULTIPARTLOG){
				this->writeJson(strev->str);
			} else {
				if(strev->totalsize==0) strev->totalsize=getJSONSize();
				strev->tobecontinued=this->writeJson(strev->str,0,strev->index,strev->maxsize);
			}
			uint32_t ts1=millis();
			println(GenString()+RF("IOLoggerv01::notify writeJson took ")+to_string(ts1-ts0)+RF("ms"));

			if(command==RF("getLogJsonCheck")){	// ??

			}

#ifdef ESP8266
			//		print("IOLoggerv01::notify post : free memory :");	println(ESP.getFreeHeap(),DEC);
#endif
			return true;
		}
		return false;
	}

	bool saveToLog(GenString cname, double dval){
		Channel* c=logs.get(cname);
		if(!c) {c=new Channel();logs.set(cname,c);}
		uint64_t ts=getMillis64();
		//		println(GenString()+"logdiff:"+to_string(((ts-c->lasttimestamp)/1000))+" "+to_string(refreshPeriodSec));
		if(c->lasttimestamp && ((ts-c->lasttimestamp)/1000)<refreshPeriodSec) return false; // got a value too soon
		c->lasttimestamp=ts;

		c->log.push_back(LogValue(ts,dval));
		if(c->log.size()>maxsize) c->log.erase(c->log.begin(),c->log.begin()+1);
		//	println("LOG:"+getJson());
#ifdef ESP8266
		//		print("IOLoggerv01::saveToLog : free memory post:");	println(ESP.getFreeHeap(),DEC);
		//		println(GenString()+"IOLoggerv01::saveToLog :"+to_string(c->log.size()));
#endif
		return true;
	}


	bool humanreadable=false;



	virtual GenString getJson(){
		GenString json;
		writeJson(json);
		return json;
	}

	unsigned long getJSONSize(){
		GenString str;
		unsigned long l=0;
		writeJson(str,&l);
		return l;
	}


	bool writeJson(GenString &json2, unsigned long *countonly=0,  unsigned int index=0, unsigned int maxsize=0){
		bool limit=true;
		if(maxsize==0) limit=false; //no limit
		GenString json=RF("{");
		bool first=true;
		for(auto it:logs){
#ifdef ESP8266
			//			print("IOLoggerv01::getJson : free memory :");	println(ESP.getFreeHeap(),DEC);
#endif
			if(!first) json+=RF(",");else first=false;
			Channel *c=it.valueRef();
			GenString key=GenString(RF("\""))+*it.key()+RF("\"");
			json+=key+RF(":[");
			bool firstdata=true;
			if(humanreadable) json+=RF("\n\t\t");
			for(auto jt:c->log){
				if(!firstdata) {
					json+=",";
					if(humanreadable) json+=RF("\n\t\t");
				} else firstdata=false;

				json+=RF("[");
				if(JSONDQ) json+=RF("\"");

				uint64_t ts;
				if(JSONMILLIS) ts=jt.getMS();
				else ts=jt.getS();
				json+=to_string(ts);
				if(JSONDQ) json+=RF("\"");
				json+=RF(",");
				if(JSONDQ) json+=RF("\"");
				json+=to_stringWithPrecision(jt.value,2);
				if(JSONDQ) json+=RF("\"");
				json+=RF("]");

				if(countonly) {*countonly+=json.size();json.erase();}
				if(!limit) {json2+=json;json.erase();}
				else{
					if((json.size()+json2.size())<maxsize) {json2+=json;json.erase();}
					else {
						if(index==0) return true;
						index--;
						json2.erase();	//skip this bloc
						json2+=json;json.erase();
					}
				}
			};
			if(humanreadable) json+=RF("\n");
			json+=RF("]");
		}
		json+=RF("}");
		if(countonly) {*countonly+=json.size();json.erase();}
		json2+=json;
		return false;
	};
};

#endif


