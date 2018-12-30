#ifndef IOLOGGERV015_H
#define IOLOGGERV015_H

#include "../../infrastructure/SyncedClock.h"
#include "../../datastruct/GenString.h"

#include "IOLoggerGen.h"

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.iologgerv15"

#define REFRESHPERIODCONST 10	//second between each reading saved
#define JSONDQ false
#define JSONMILLIS false

//#define TS64
#define TS32
#define MILLISFACTOR 1000;	//use seconds
class IOLoggerv015: public IOLoggerGen
{
	// save the value only every some period drop them otherwise
	// another step is to ask to refresh the sensor (with a timer)

	typedef unsigned char ColumnType;
	typedef float ValType;
	struct __attribute((__packed__)) ValCell{
		ColumnType column;
		ValType val;
	};
	typedef unsigned char StreamLengthType;
	typedef uint32_t TimeStampType;
	struct StreamCell{
		TimeStampType ts=0;
		StreamLengthType length=0;
		ValCell *cells=0;
		StreamCell(const StreamCell& obj){
			this->ts=obj.ts;this->length=obj.length;
			cells=new ValCell[length];
			for(unsigned int i=length;i>0;i--) this->cells[length-i]=obj.cells[length-i];
		}
		StreamCell():length(){}
		~StreamCell(){if(cells) {delete cells;cells=0;}}
		void pushCell(ValCell vcell){
			if(!length) {length++;
			println(GenString()+RF("StreamCell::pushCell : size of ValCell :")+to_string(sizeof(ValCell))+RF(" ")+to_string(sizeof(ValCell[length])));
			println(GenString()+RF("StreamCell::pushCell : length :")+to_string(length));
#ifdef ESP8266
	 				print(RF("StreamCell::pushCell : free memory push allocation :"));	println(ESP.getFreeHeap(),DEC);
#endif
	 		cells=new ValCell[length];	// 16 bytes minimum !

#ifdef ESP8266
	 				print(RF("StreamCell::pushCell : free memory push post allocation :"));	println(ESP.getFreeHeap(),DEC);
#endif

	 		println(RF("cells:")+to_string((uint64_t)cells));
	 		cells++;
	 		println(RF("cells:")+to_string((uint64_t)cells));
	 		cells--;
			}
			else {// copy into bigger array
				length++;
#ifdef ESP8266
	 				print(RF("StreamCell::pushCell : free memory push reallocation :"));	println(ESP.getFreeHeap(),DEC);
#endif
	 			println(GenString()+RF("StreamCell::pushCell : length :")+to_string(length));
				ValCell *ncells=new ValCell[length];
#ifdef ESP8266
	 				print(RF("StreamCell::pushCell : free memory push post reallocation :"));	println(ESP.getFreeHeap(),DEC);
#endif
				for(unsigned char i=0;i<length-1;i++) ncells[i]=cells[i];
				delete cells;
				cells=ncells;
			}
			cells[length-1]=vcell;
		};
		void resync(uint64_t diffms){ts+=diffms/MILLISFACTOR;}
	};
	struct Channel {
		ColumnType column;
		TimeStampType lastts=0;
	};

	GenObjMap<GenString, Channel*> channels;
	std::vector<StreamCell> logdata;
	std::vector<StreamCell> pending;


	unsigned char columncount=0;

	bool locked=false;
	TimeStampType savedts=0,savedfirstts=0;

	unsigned int maxsize=6*24*3;
	unsigned int minmemory=12000;

	unsigned int refreshPeriodSec;	// storing and using ms when refresh is e.g. 10 seconds is useless

	class Logv015Syncer : public ClockListener{
		IOLoggerv015 *mylogger;
	public:
		Logv015Syncer(IOLoggerv015*mylogger0): mylogger(mylogger0){};
		virtual void clockresync(uint64_t diffms){
			println(RF("LogSyncer:")+to_string(diffms));
			if(mylogger) mylogger->resync(diffms);
		};
	};

	public:

	void resync(uint64_t diffms){
		if(savedts) savedts+=diffms/MILLISFACTOR;
		for(std::vector<StreamCell>::iterator it=logdata.begin();it!=logdata.end();it++) it->resync(diffms);	// correct each timestamp
	}

	IOLoggerv015 (GenMap *config0): IOLoggerGen(config0){
		refreshPeriodSec=REFRESHPERIODCONST;
				if(config0) {
					GenString str=config0->get(RF(LOGREFRESHPERIOD_KEYWORD));
					if(isDigit(str)) refreshPeriodSec=strToDouble(str);
				}
		println(RF("Constructor IOLoggerv015")); };

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
			if(strev->index==0 && locked) return false;	// cant make a new during lock
			strev->str.erase();
		//	uint32_t ts0=millis();
			if(!MULTIPARTLOG){
				this->writeJson(strev->str);
			} else {
				// so far we wont use the size
				//				if(strev->totalsize==) strev->totalsize=getJSONSize();
				locked=true;
				strev->tobecontinued=this->writeJson(strev->str,strev->maxsize);
				locked=strev->tobecontinued;
				if(!locked) {
					transferPending();
					println(GenString()+RF("IOLoggerv015::notify post : total dots :")+to_string(logdata.size()));
				}
			}
		//	uint32_t ts1=millis();
		//	println(GenString()+"IOLoggerv015::notify writeJson took "+to_string(ts1-ts0)+"ms");

#ifdef ESP8266
	//				print("IOLoggerv015::notify post : free memory :");	println(ESP.getFreeHeap(),DEC);
#endif
			return true;
		}
		return false;
	}

	void transferPending(){
		while(!pending.empty()){
			StreamCell sc=pending.front();
			saveCellToLog(sc.ts,getChannelFromColumn(sc.cells[0].column),sc.cells[0].val);
		}
	}

	bool saveToLog(GenString cname, double dval){

		uint64_t ts=getMillis64()/MILLISFACTOR;
		unsigned int mf=MILLISFACTOR;
		Channel* c=channels.get(cname);							// find channel
		if(!c) {c=new Channel();c->column=columncount++;channels.set(cname,c);}	// or create one
		uint64_t diff=(ts-c->lastts)+1;
		diff=diff*mf/1000;
		//  println(GenString()+"logdiff:"+to_string(diff)+" "+to_string(refreshPeriodSec));
		if(c->lastts && (diff)<refreshPeriodSec) return false; // got a value too soon

		if(locked) {
			ValCell vc;
			vc.column=c->column;
			vc.val=dval;
			StreamCell sc;
			sc.ts=ts;
			sc.pushCell(vc);
			pending.push_back(sc);
			return true;
		}
		return saveCellToLog(ts,c,dval);
	}
	Channel *getChannelFromColumn(ColumnType col){
		for(auto c:channels) if((*c.itval)->column==col) return *c.itval;
		return 0;
	};

	bool saveCellToLog(uint64_t ts, Channel *c, double dval ){

		// find current ts in vector
		unsigned int i=logdata.size();

		for(;i>0;i--) if(logdata[i-1].ts<=ts) break;
#ifdef ESP8266
	 				print(RF("IOLoggerv015::saveCellToLog : free memory init :"));	println(ESP.getFreeHeap(),DEC);
#endif

		ValCell vc;
		vc.column=c->column;
		vc.val=(float)dval;
#ifdef ESP8266
	 				print(RF("IOLoggerv015::saveCellToLog : free memory vc :"));	println(ESP.getFreeHeap(),DEC);
#endif
		StreamCell sc;
		sc.ts=ts;
		sc.pushCell(vc);
		if(ts>c->lastts) c->lastts=ts;
#ifdef ESP8266
	 				print(RF("IOLoggerv015::saveCellToLog : free memory sc :"));	println(ESP.getFreeHeap(),DEC);
#endif
		if(i>0){
			if(logdata[i-1].ts==ts)	{	// found cell with same ts

				logdata[i-1].pushCell(vc);
#ifdef ESP8266
	 				print(RF("IOLoggerv015::saveCellToLog : free memory update :"));	println(ESP.getFreeHeap(),DEC);
#endif
				return true;
			} else if(i==logdata.size()){	// last cell has smaller ts
				logdata.push_back(sc);
#ifdef ESP8266
	 				print(RF("IOLoggerv015::saveCellToLog : free memory push :"));	println(ESP.getFreeHeap(),DEC);
#endif
				return true;
			}
		}
		// other cases
		logdata.insert(logdata.begin()+i,sc);	// do not update c->lastcell
#ifdef ESP8266
	 				print(RF("IOLoggerv015::saveCellToLog : free memory insert :"));	println(ESP.getFreeHeap(),DEC);
#endif
		while(maxsize>0 && logdata.size()>maxsize) logdata.erase(logdata.begin());
#ifdef ESP8266
		unsigned int mem=ESP.getFreeHeap();
		while(minmemory>0 && mem<minmemory && !logdata.empty()) {logdata.erase(logdata.begin()); mem=ESP.getFreeHeap();}
		println(GenString()+RF("saveCellToLog:memory left ")+to_string(mem));
#endif
		return true;

	}

	/*
	bool humanreadable=false;

	unsigned long getJSONSize(){
		GenString str;
		unsigned long l=0;
		writeJson(str,&l);
		return l;
	}
	 */

	virtual GenString getJson(){
		GenString json;
		writeJson(json);
		return json;
	}


	bool writeJson(GenString &json2, unsigned int maxsize=0){
		bool limit=true;
		if(maxsize==0) limit=false; //no limit
		GenString json;

		unsigned char maxcol=0;
		for(GenObjMap<GenString,Channel*>::Iterator it:channels)	{
			Channel *c=*it.itval;
			if(c->column>maxcol) maxcol=c->column;
		}
		if(savedts==0){
			 json+=RF("[[");
			bool first=true;
			for(unsigned int i=0;i<=maxcol;i++){// add column names
				GenString name;
				for(GenObjMap<GenString,Channel*>::Iterator it:channels)	{
					Channel *c=*it.itval;
					if(c->column==i) name=*it.itkey;
				}
				if(!first) json+=RF(",");
				first=false;
				json+=RF("\"")+name+RF("\"");
			}
			json+="]";
		}
		for(StreamCell c:logdata){
			if(c.ts<savedts) continue;
			json+=RF(",[");
			uint64_t ts=c.ts;
			if(!savedfirstts) savedfirstts=ts;
			else {ts-=savedfirstts;}
			json+=to_string(ts);
			for(unsigned int i=0;i<c.length;i++){
				unsigned int col=c.cells[i].column;
				json+=GenString()+","+to_string(col)+",";
				json+=to_stringWithPrecision(c.cells[i].val,2);
			}
			json+="]";
			if(!limit || (json.size()+json2.size())<maxsize) {json2+=json;json.erase();}
			else {
				savedts=c.ts;
				return true;
			}

		}
		json+=RF("]");
		json2+=json;json.erase();
		savedts=0;
		savedfirstts=0;
		return false;
	};
};


#endif

