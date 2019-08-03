#ifndef IODATAV01_H
#define IODATAV01_H

#include "IODataGen.h"

#include "../../infrastructure/CompatPrint.h"



GenString getMin(GenString,GenString);
GenString getMax(GenString,GenString);
GenString getPathLeaf(GenString path);
GenString getPathBranch(GenString path);
bool isDigit(GenString str);

class IODatav01 : public IODataGen
{
public:
	bool update(GenString path, GenMap map){
		bool b=data.update(path,map);

		GenString val=map.get(VALUE_FIELD);	// if value was modified update minmax
		if(!val.empty()) updateMinMax(path,val);

		// should notify only the fields that really changed value, e.g. diffmap=oldmap-newmap;
		emitChange(path,map);

		return b;
	};
	void emitChange(GenString &path, GenMap &map){
		NamedStringMapEvent e(path,&map) ;
		emit(MODEL_UPDATED_EVENT, &e);
	};

	bool updateVal(GenString path, GenString val, bool notifychange=true){
		bool b= data.updateVal(path,val);
		GenString pls=getPathLeaf(path);
		GenString br=getPathBranch(path);
		if(pls==VALUE_FIELD) updateMinMax(br,val); // if value was modified update minmax
		GenMap map={{pls,val}};
		//if(pls==TSFIELD) map.set(VALUEFIELD,get(br+"/"+VALUEFIELD));	//we think having
		if(notifychange) emitChange(br,map);
		return b;
	};


	bool updateMinMax(GenString path, GenString val){
		if(!isDigit(val)) return false;
		bool b=false;
		GenString omin=data.get(path+SLASH+MIN_FIELD);
		GenString omax=data.get(path+SLASH+MAX_FIELD);
		GenString min=getMin(omin,val);
		GenString max=getMax(omax,val);
		//			println("Omin:"+omin);
		//			println("Omax:"+omax);
		if(min!=omin) {data.updateVal(path+SLASH+MIN_FIELD,min);b=true;}	// should trigger a notification at some point
		if(max!=omax) {data.updateVal(path+SLASH+MAX_FIELD,max);b=true;}
		return b;
	}

	virtual GenString get(GenString path){
		return data.get(path);
	}

	virtual bool notify(GenString ename,Event*event=0){

		if(ename==GET_JSON_DATA_EVENT){
			StringEvent *strev=0;
			if(event->getClassType()==StringEventTYPE) strev = (StringEvent*)(event);
			if(!strev) return false;
			strev->str=this->getAsJson();
			return true;
		}

		if(ename==UPDATE_MODEL_EVENT){

			//	println("IODatav01:notify UPDATEMODEL");
			StringMapEvent *emap=0;
			if(event->getClassType()==StringMapEventTYPE) emap=(StringMapEvent*)(event);//should check type before casting
			if(!emap || emap->values.empty()) return false; //pb
			//	auto it=emap->values.begin();
			//for(;!it.isEnd();it++){
			for(GenMap::Iterator it=emap->values.begin();!it.isEnd();it++) {
				updateVal(it.key(),it.value(),false);
			}
			//should emit one change
			emit(MODEL_UPDATED_EVENT, emap);
			//println("IODatav01:notify :"+ename+" "+cell.key+" "+cell.value);
			println(GenString()+RF("updated model to:")+getAsJson());

			return true;
		}

		return false;
	};

	GenString getAsJson(){return data.getAsJson();}
};

#endif


