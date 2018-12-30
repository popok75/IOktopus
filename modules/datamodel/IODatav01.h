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

		GenString val=map.get(VALUEFIELD);	// if value was modified update minmax
		if(!val.empty()) updateMinMax(path,val);

		// should notify only the fields that really changed value, e.g. diffmap=oldmap-newmap;
		emitChange(path,map);

		return b;
	};
	void emitChange(GenString &path, GenMap &map){
		NamedStringMapEvent e(path,&map) ;
		emit(MODELUPDATED, &e);
	};

	bool updateVal(GenString path, GenString val){
		bool b= data.updateVal(path,val);
		GenString pls=getPathLeaf(path);
		GenString br=getPathBranch(path);
		if(pls==VALUEFIELD) updateMinMax(br,val); // if value was modified update minmax
		GenMap map={{pls,val}};
		emitChange(br,map);
		return b;
	};


	bool updateMinMax(GenString path, GenString val){
			if(!isDigit(val)) return false;
			bool b=false;
			GenString omin=data.get(path+SLASH+MINFIELD);
			GenString omax=data.get(path+SLASH+MAXFIELD);
			GenString min=getMin(omin,val);
			GenString max=getMax(omax,val);
//			println("Omin:"+omin);
//			println("Omax:"+omax);
			if(min!=omin) {data.updateVal(path+SLASH+MINFIELD,min);b=true;}	// should trigger a notification at some point
			if(max!=omax) {data.updateVal(path+SLASH+MAXFIELD,max);b=true;}
			return b;
		}

	virtual GenString get(GenString path){
		return data.get(path);
	}

	virtual bool notify(GenString ename,Event*event=0){

		if(ename==GETASJSON){
			StringEvent *strev=0;
			if(event->getClassType()==StringEventTYPE) strev = (StringEvent*)(event);
			if(!strev) return false;
			strev->str=this->getAsJson();
			return true;
		}

		if(ename==UPDATEMODEL){

	//		println("IODatav01:notify");
			StringMapEvent *emap=0;
			 if(event->getClassType()==StringMapEventTYPE) emap=(StringMapEvent*)(event);//should check type before casting
			if(!emap || emap->values.empty()) return false; //pb
		//	auto it=emap->values.begin();
			//for(;!it.isEnd();it++){

			for(GenMap::Iterator it=emap->values.begin();
					!it.isEnd();
					it++)
				{

				updateVal(it.key(),it.value());

				}
			//should emit one change
			//println("IODatav01:notify :"+ename+" "+cell.key+" "+cell.value);
			println(GenString()+RF("updated model to:")+getAsJson());

 			return true;
		}

		return false;
	};
};

#endif


