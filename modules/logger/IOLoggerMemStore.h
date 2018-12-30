#ifndef IOLOGGERMEMSTORE_H
#define IOLOGGERMEMSTORE_H


#include "../../datastruct/ChunkedVector.h"
#include "../../datastruct/GenString.h"

#include "IOLoggerGen.h"


typedef unsigned char ColumnType;
typedef float ValType;
typedef uint32_t TimeStampType;
typedef uint16_t PositionType;

#define MILLISFACTOR 1000;	//use seconds

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.iologgermemstore"

class IOLoggerMemStore
{
	// save the value only every some period drop them otherwise
	// another step is to ask to refresh the sensor (with a timer)

	struct __attribute((__packed__)) ValCell{
		ColumnType column=0;
		ValType val=0;
		ValCell(ColumnType col0,ValType val0):column(col0),val(val0){};
		ValCell(){};
	};
	struct __attribute((__packed__)) TSCell{
		TimeStampType ts=0;
		PositionType pos=0;
		ColumnType length=0;
		TSCell(TimeStampType ts0,PositionType pos0, ColumnType length0):ts(ts0),pos(pos0),length(length0){};
		TSCell(){};
	};

	struct Channel {
		ColumnType column;
		TimeStampType lastts=0;
	};

	struct PendingCell {
		ColumnType column=0;
		TimeStampType ts=0;
		ValType val=0;
		PendingCell(ColumnType col0,ValType val0,TimeStampType ts0):column(col0),ts(ts0),val(val0){};
	};


	ChunkedVector<TSCell> timestamps;
	ChunkedVector<ValCell> vals;
	std::vector<PendingCell> pending;
	GenObjMap<GenString, Channel*> channels;

	unsigned char columncount=0;

	bool locked=false;

public:
	unsigned int sizelimit=1000;
	unsigned int minmemory=16000;

	struct FullCell {
		TimeStampType ts=0;
		std::vector<ValCell> cells;
		FullCell(TimeStampType ts0,unsigned int length):ts(ts0), cells(length){}
		FullCell(){};
	};



	IOLoggerMemStore (){println(RF("Constructor IOLoggerMemStore")); };

	void resync(uint64_t diffms){
		//		if(savedts) savedts+=diffms/MILLISFACTOR;
		//		for(std::vector<StreamCell>::iterator it=logdata.begin();it!=logdata.end();it++) it->resync(diffms);	// correct each timestamp
	}
	unsigned int size(){return timestamps.size();}

	std::vector<GenString>getColumnNames(){
		std::vector<GenString> vect(channels.size());

		for(GenObjMap<GenString,Channel*>::Iterator it:channels){
			Channel *c=*it.itval;
			vect[c->column]=*(it.itkey);
		}
		return vect;
	}

	FullCell getIndex(unsigned int i){
		if(i>timestamps.size()) return FullCell();
		FullCell ts(timestamps[i].ts,timestamps[i].length);
		for(unsigned int j=0;j<timestamps[i].length;j++) ts.cells[j]=vals[timestamps[i].pos+j];
		return ts;
	}

	void setLocked(bool b){
		if(locked && !b) {
			locked=b;
			transferPending();
	//		println(GenString()+RF("IOLoggerMemStore::setLocked : total timestamp dots :ts:")+to_string(timestamps.size())+RF(" ,val:")+to_string(vals.size()));
		} else locked=b;
	}
	bool isLocked(){return locked;}

	uint64_t getLastTSMS(GenString cname){

		Channel* c=channels.get(cname);
	//	println(GenString()+RF("IOLoggerMemStore::getLastTSMS : cname ")+cname+RF(" ")+to_string((uint64_t)c));
		if(!c) return 0;
//		println(GenString()+RF("IOLoggerMemStore::getLastTSMS : c->lastts :")+to_string((uint64_t)c->lastts));
		uint64_t ts=c->lastts;
		ts=ts*MILLISFACTOR
		return ts;
	}

	void transferPending(){
		while(!pending.empty()){
			PendingCell sc=pending.front();
			saveCellToLog(sc.ts,getChannelFromColumn(sc.column),sc.val);
			pending.erase(pending.begin());
		}
	}

	bool addData(GenString cname, double dval, uint64_t tsms){
		uint64_t ts=tsms/MILLISFACTOR;
		//		unsigned int mf=MILLISFACTOR;
		Channel* c=channels.get(cname);							// find channel
		if(!c) {c=new Channel();c->column=columncount++;channels.set(cname,c);}	// or create one
		if(locked) {
			pending.push_back(PendingCell(c->column,dval,ts));
			return true;
		}

		return saveCellToLog(ts,c,dval);
	//	return false;
	}

	Channel *getChannelFromColumn(ColumnType col){
		for(auto c:channels) if((*c.itval)->column==col) return *c.itval;
		return 0;
	};

#define EXTRA_ALLOCATE 4

	bool saveCellToLog(TimeStampType ts, Channel *c, double dval ){

//		println(GenString()+RF("saveCellToLog start timestamps size:")+to_string(timestamps.size()));

#ifdef ESP8266BUILD
//		println(GenString()+RF("---IOLoggerMemStore::saveCellToLog - memory at start:")+to_string(ESP.getFreeHeap()));
#endif
		// find current ts in vector
		unsigned int i=timestamps.size();

		for(;i>0;i--) if(timestamps[i-1].ts<=ts) break;

		ValCell vc(c->column,(float)dval);

		if(ts>c->lastts) c->lastts=ts;

		if(i>0){
			if(timestamps[i-1].ts==ts)	{	// found cell with same ts// if already existing cell with same ts
 				TSCell tscell=timestamps[i-1];
				unsigned int index=tscell.pos+tscell.length;
#ifdef ESP8266BUILD
	//			println(GenString()+RF("---IOLoggerMemStore::saveCellToLog - memory pre insert:")+to_string(ESP.getFreeHeap()));
#endif
		//		if(!(vals.size()%EXTRA_ALLOCATE)) vals.reserve(vals.size()+EXTRA_ALLOCATE);
		//		vals.insert(vals.begin()+index,vc);	//=> insert vals at following position
				vals.insert(index,vc);
#ifdef ESP8266BUILD
	//			println(GenString()+RF("---IOLoggerMemStore::saveCellToLog - memory post insert:")+to_string(ESP.getFreeHeap()));
#endif
		//		if(!(timestamps.size()%EXTRA_ALLOCATE)) timestamps.reserve(timestamps.size()+EXTRA_ALLOCATE);
				timestamps[i-1].length++;
				for(unsigned int j=i;j<timestamps.size();j++) timestamps[j].pos++; //(eventually correct all following tscells)
				eraseNeeded();
//				println(GenString()+RF("at end timestamps size:")+to_string(timestamps.size()));
	 			return true;
			}  else if(i==timestamps.size()){	// last cell has smaller ts
				TSCell tsc(ts,vals.size(),1);
#ifdef ESP8266BUILD
	//			println(GenString()+RF("---IOLoggerMemStore::saveCellToLog - memory pre push11:")+to_string(ESP.getFreeHeap()));
#endif
			//	if(!(vals.size()%EXTRA_ALLOCATE)) vals.reserve(vals.size()+EXTRA_ALLOCATE);
				vals.push_back(vc);
#ifdef ESP8266BUILD
	//			println(GenString()+RF("---IOLoggerMemStore::saveCellToLog - memory pre push12:")+to_string(ESP.getFreeHeap()));
#endif
			//	if(!(timestamps.size()%EXTRA_ALLOCATE)) timestamps.reserve(timestamps.size()+EXTRA_ALLOCATE);
				timestamps.push_back(tsc);
#ifdef ESP8266BUILD
	//			println(GenString()+RF("---IOLoggerMemStore::saveCellToLog - memory post push1:")+to_string(ESP.getFreeHeap()));
#endif
				eraseNeeded();
//				println(GenString()+RF("at end timestamps size:")+to_string(timestamps.size()));
				return true;
			}
		}
#ifdef ESP8266BUILD
//		println(GenString()+RF("---IOLoggerMemStore::saveCellToLog - memory pre push21:")+to_string(ESP.getFreeHeap()));
#endif
	//	if(!(vals.size()%EXTRA_ALLOCATE)) vals.reserve(vals.size()+EXTRA_ALLOCATE);
		vals.push_back(vc);
		TSCell tsc(ts,vals.size()-1,1);
		// other cases
#ifdef ESP8266BUILD
//		println(GenString()+RF("---IOLoggerMemStore::saveCellToLog - memory pre push22:")+to_string(ESP.getFreeHeap()));
#endif
	//	if(!(timestamps.size()%EXTRA_ALLOCATE)) timestamps.reserve(timestamps.size()+EXTRA_ALLOCATE);
	//	timestamps.insert(timestamps.begin()+i,tsc);
//		println(GenString()+RF("preinsert timestamps size:")+to_string(timestamps.size()));
		timestamps.insert(i,tsc);
//		println(GenString()+RF("postinsert timestamps size:")+to_string(timestamps.size()));
#ifdef ESP8266BUILD
//		println(GenString()+RF("---IOLoggerMemStore::saveCellToLog - memory post push2:")+to_string(ESP.getFreeHeap()));
#endif
	 	eraseNeeded();
	 	println(GenString()+RF("---IOLoggerMemStore::saveCellToLogat end timestamps size:")+to_string(timestamps.size()));
		return true;
	}

	void eraseNeeded(){
	 /*	if(timestamps.size()>1){
			auto *p1=&timestamps[timestamps.size()-1];
			auto *p2=&timestamps[timestamps.size()-2];
			println( GenString()+RF("p1:")+to_string((uint64_t)p1)+RF(" p2:")+to_string((uint64_t)p2));
			auto *p3=&vals[vals.size()-1];
			auto *p4=&vals[vals.size()-2];
			println( GenString()+RF("p3:")+to_string((uint64_t)p3)+RF(" p4:")+to_string((uint64_t)p4));
		}*/
//		println(GenString()+RF("eraseNeeded step1 timestamps size:")+to_string(timestamps.size()));
		while(sizelimit>0 && timestamps.size()>sizelimit) {
			eraseFirst();
			println(GenString()+RF("saveCellToLog:eraseNeeded erased one, timestamp size ")+to_string(timestamps.size()));
	//		println(GenString()+RF("IOLoggerMemStore::eraseNeeded:memory erasing item1,timestamp size ")+to_string(timestamps.size()));

		}
//		println(GenString()+RF("eraseNeeded step2 timestamps size:")+to_string(timestamps.size()));
#ifdef ESP8266
		unsigned int mem=ESP.getFreeHeap();
		while(minmemory>0 && mem<minmemory && !timestamps.empty()) {
//			println(GenString()+RF("eraseNeeded step2.5 timestamps size:")+to_string(timestamps.size()));
			eraseFirst();
//			println(GenString()+RF("eraseNeeded step2.6 timestamps size:")+to_string(timestamps.size()));
//			println(GenString()+RF("eraseNeeded step2.7 timestamps empty:")+to_string(timestamps.empty()));
			mem=ESP.getFreeHeap();
			println(GenString()+RF(">>>>>>>>>IOLoggerMemStore::eraseNeeded:memory erasing item, mem left ")+to_string(mem)+RF(" items:")+to_string(timestamps.size()));
		}
	//	println(GenString()+RF("IOLoggerMemStore::eraseNeeded:memory left ")+to_string(mem));
#endif
//		println(GenString()+RF("eraseNeeded step3 timestamps size:")+to_string(timestamps.size()));
	}

	void eraseFirst(){
//		println(GenString()+RF("eraseFirst step 0 timestamps size:")+to_string(timestamps.size()));
		TSCell front=timestamps.front();
//		println(GenString()+RF("eraseFirst step 1 timestamps size:")+to_string(timestamps.size()));
		unsigned int l=front.length;
//		println(GenString()+RF("eraseFirst step 2 timestamps size:")+to_string(timestamps.size()));
//		println(GenString()+RF("eraseFirst step 2 ")+to_string(vals.begin()+ front.pos)+" "+to_string(vals.begin()+front.pos+l));

		vals.erase(vals.begin()+ front.pos, vals.begin()+front.pos+l);
//		println(GenString()+RF("eraseFirst step 3 timestamps size:")+to_string(timestamps.size()));
		timestamps.erase(timestamps.begin());
//		println(GenString()+RF("eraseFirst step 4 timestamps size:")+to_string(timestamps.size()));
		for(unsigned int i=0;i<timestamps.size();i++) timestamps[i].pos-=l;
//		println(GenString()+RF("eraseFirst step 5 timestamps size:")+to_string(timestamps.size()));

	};


};


#endif



