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

#define NODESPATH "/nodes/"
#define VALKEYWORD "val"
#define TSKEYWORD "ts"

#define MODELUPDATEDEVENT "modelUpdated"
#define GETJSONLOGEVENT "getLogJson"

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
		if(command==RF(MODELUPDATEDEVENT)){
			StringMapEvent *evmap=0;
			if(event->getClassType()==StringMapEventTYPE) evmap=(StringMapEvent*)(event);
			if(!evmap) return false;
			GenString vnamesave;

			// find a val, check if we have the ts, save it
			std::vector<GenString> loaded ;
			// extract first var name
				// see if we have a val and ts -> save
			GenString nodespath=RF(NODESPATH);
			for(auto it : evmap->values){
				GenString key=it.key();
				GenString vname=getPathBranch(key).erase(0,nodespath.size());
				bool newvar=true;
				for(GenString ls : loaded) if(ls==vname) {newvar=false;break;}
				if(!newvar) continue;
				println(GenString()+key);
				println(GenString()+vname);
				GenString valstr=evmap->values.get(nodespath+vname+'/'+RF(VALKEYWORD));
				if(valstr.empty()) continue;
				GenString tsstr=evmap->values.get(nodespath+vname+'/'+RF(TSKEYWORD));
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
/*

	 		NamedStringMapEvent *namev=0;
			if(event->getClassType()==NamedStringMapEventTYPE) namev=(NamedStringMapEvent*)(event);
			if(!namev) return false;

			if(startsWith(namev->ename,RF(NODESPATH)) && namev->values.has(RF("ts"))){	//log only nodes // at ts change, val is sent too
				GenString strval=namev->values.get(RF("val"));
				if(isDigit(strval)){
					double dval=strToDouble(strval);
					GenString tsstr=namev->values.get(RF("ts"));
					uint64_t nts=0;
					if(isDigit(strval)) {
						nts=strToUint64(tsstr);
					}
					saveToLog(getPathLeaf(namev->ename),dval,nts);
				} else return false;
			}*/
		}
		if(command==RF(GETJSONLOGEVENT)){
			NamedStringMapEvent *strev=0;
			if(event->getClassType()==NamedStringMapEventTYPE) strev=(NamedStringMapEvent*)(event);

			unsigned int maxsize=0;
			GenString max=strev->values.get(RF("max"));
			if(isDigit(max)) maxsize=strToUint64(max);
			uint64_t timestamp=0;
			GenString ts=strev->values.get(RF("ts"));
			if(isDigit(ts)) timestamp=strToUint64(ts);

			GenString cont=RF("tobecontinued");
			bool tobecontinued=strev->values.has(cont);
			if(!strev) return false;
			if(!tobecontinued && logdata.isLocked()) return false;	// cant make a new during lock
			strev->ename.erase();
			if(timestamp) {
				unsigned int i=logdata.getIndexFromTS(timestamp);//savedindex should be controlled only from writeJson funciton, not good to hack it as argument of call
				if(i==logdata.size()) {strev->ename=RF("[]");strev->values.set("total","2");return true;}//nothing new
				savedindex=i;
			}
			logdata.setLocked(true);

			tobecontinued=this->writeJson(strev->ename,maxsize);
			if(tobecontinued) strev->values.set(cont,RF("1"));
			else strev->values.erase(cont);
			logdata.setLocked(tobecontinued);
			if(!tobecontinued) println(GenString()+RF("IOLoggerv016::notify post : total dots :")+to_string(logdata.size()));
		//	println(GenString()+RF("IOLoggerv016::notify json:")+(strev->str));
#ifdef ESP8266
			//				print("IOLoggerv016::notify post : free memory :");	println(ESP.getFreeHeap(),DEC);
#endif
			return true;
		}
		return false;
	}



	bool saveToLog(GenString cname, double dval,uint64_t tsdata){
#ifdef ESP8266
 	print(RF("IOLoggerv016::saveToLog - memory :"));	 println(ESP.getFreeHeap(),DEC);
#endif
		uint64_t ts=tsdata;
		if(!ts) ts=getMillis64();
		uint64_t diff=logdata.getLastTSMS(cname);
//		println(GenString()+RF("IOLoggerMemStore::saveToLog : new ts ")+to_string(ts)+RF(" saved ts:")+to_string(diff));
		if(diff) {
			//		println(GenString()+RF("IOLoggerMemStore::saveToLog : new ts ")+to_string(ts)+RF(" saved ts:")+to_string(diff));
			diff=ts-diff;
			diff=diff/1000;
		//    println(GenString()+RF("logdiff:")+to_string(diff)+RF(" ")+to_string(refreshPeriodSec));
			if((diff+1)<refreshPeriodSec) return false; // got a value too soon
		}
		println(GenString()+"adding data : "+cname+ " v:"+to_stringWithPrecision(dval,2)+" ts:"+to_string(ts));
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
			json+=RF("[[\"newstream\"],{");
			bool first=true;
			auto names=logdata.getColumnNames();
			for(unsigned int i=0;i<names.size();i++){
				GenString name=names[i];
				if(!first) json+=RF(",");
				first=false;
				json+=RF("\"")+name+RF("\":")+to_string(i);
			}
			json+=RF("}");
		}


		unsigned int dots=logdata.size();
//		println(GenString()+RF("IOLoggerv016::writeJson dots:")+to_string(logdata.size()));
		for(unsigned int i=savedindex;i<dots;i++){
//			println(GenString()+RF("IOLoggerv016::writeJson loop json:")+json+" json2:"+json2);
			IOLoggerMemStore::FullCell fc=logdata.getIndex(i);
			uint64_t ts=fc.ts;
			if(!savedfirstts) {
				savedfirstts=ts;
				json+=RF(",{\"sync\":")+to_string(ts)+'}';
			}
			ts-=savedfirstts;	//
			json+=RF(",[");
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

