#ifndef IODATAV03_H
#define IODATAV03_H

#include "IODataEmitter.h"

/*	IODatav03
 * 		This is the central class of the model.
 * 		Tree : It allow to store and retreive string data in a tree structure using paths (like files).
 * 		In IODataEmitter.h, find detail about Links and Event propagation.
 *
 * */

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.iodatav03"


#define WRITETAG "write"


////////////////////////////////// IOData
class IODatav03;
class IODataPlugin {
protected:
	IODatav03 *datamodel;
public:
	//	void init(){}; //called at iodata init
	//	void update(){}; //called at iodata update
	void setModel(IODatav03*datamodel1){datamodel=datamodel1;}

};







///////////////////////////////////////// IODATAV03 class
class IODatav03 : public IODataEmitter {	// IODataGen is inherited by IODataEmitter that need the emitter interface
	GenTreeMap data;
	std::vector<IODataPlugin*> plugins;//could be a list

public:
	~IODatav03(){for(IODataPlugin* plugin : plugins) {delete plugin;}}

//	bool notify(GenString path, Event *){return true;};	//TODO: implement this

	virtual bool notify(GenString ename,Event*event=0){ // save as v01, should be externalized somewhere practical

			if(ename==GET_JSON_DATA_EVENT){
				StringEvent *strev=0;
				if(event->getClassType()==StringEventTYPE) strev = (StringEvent*)(event);
				if(!strev) return false;
				strev->str=this->getAsJson();
				return true;
			}/*
			if(ename==GET_DATA_SUBVALUES_EVENT){  // strev->str is the path argument
				StringMapEvent *emap=0;
				if(event->getClassType()==StringMapEventTYPE) emap=(StringMapEvent*)(event);
				if(!emap || emap->values.empty()) return false;
				GenString path=emap->values.begin().value();
			//	if(!path) return false;
				path="/operations/MaxHumidity/dst/";

				bool hp=hasPath(path);
				std::vector<GenString> list=subPaths(path);
				for(GenString k : list){
					println(GenString()+"subkey: "+k);
				}

				return true;
			}*/
/*			if(ename==GET_DATA_NODE_EVENT){
				StringMapEvent *emap=0;
				if(event->getClassType()==StringMapEventTYPE) emap=(StringMapEvent*)(event);
				if(!emap || emap->values.empty()) return false;
				GenString path=emap->values.begin().value();
				...we need a function to read a whole node
				return true;
			}*/

			if(ename==UPDATE_MODEL_EVENT){
				//	println("IODatav01:notify UPDATEMODEL");
				StringMapEvent *emap=0;
				if(event->getClassType()==StringMapEventTYPE) emap=(StringMapEvent*)(event);// check type before casting
				if(!emap || emap->values.empty()) return false; //pb
				//	auto it=emap->values.begin();
				//for(;!it.isEnd();it++){
				bool b=false;
				for(GenMap::Iterator it : emap->values) {
					println(GenString()+"key: "+it.key());
					b=updateVal(it.key(),it.value(),true) || b;
				}
				// emit one change
				PtrNStringMapEvent ptremap=PtrNStringMapEvent((void*)&data, &emap->values);
				emit(MODEL_UPDATED_EVENT, &ptremap);	// 2 notification system : - tag based ("modelUpdate",Eventmap), vs path+tag ("/nodes/in/value#write")
				//println("IODatav01:notify :"+ename+" "+cell.key+" "+cell.value);
		//		println(GenString()+RF("updated model to:")+getAsJson());
				emap->ok=b;
				return true;
			}

			return false;
		};


	bool update(GenString path, GenMap map){
		bool b=false;
		for(GenMapProto::Iterator it:map){
			b=b||data.updateVal(path+'/'+it.key(),it.value());
		}
		//Min Max calculation
		//GenString val=map.get(VALUEFIELD);	// if value was modified update minmax
		//if(!val.empty()) updateMinMax(path,val);

		// should notify only the fields that really changed value, e.g. diffmap=oldmap-newmap;
	//	emitChange(path,map);

		return b;
	};
	//	virtual GenString get(GenString path){return "";};

	GenString getDataVal(GenString path){ return data.get(path);};	// get raw Data value without link traversal, etc, equivaluent to getVal("#/somepath/")

	GenString get(GenString path){
		GenString npath=resolveLink(path);
		GenString val=getDataVal(npath);
		return val;// or else just return the value
	};

	bool hasPath(GenString path){return data.hasPath(path);}

	std::vector<GenString> subPaths(GenString path){return data.listSubpaths(path);}	// direct subkeys
	GenString progeny(GenString path,GenString startafter=""){return data.findProgeny(path,startafter);}	// direct subkeys

	GenString searchValue(GenString value, GenString startpath=""){	return data.findValuePath(value,startpath);};


//	bool updateVal(GenString path, GenString val){return updateVal(path,val,true);}
	bool updateVal(GenString path, GenString val, bool notifychange=true){

		if(path=="delete") return data.eraseSubPaths(val);

	 	// method for modificiation without link follow : # after path
		GenString npath=resolveLink(path);

	//	std::cout << "IODatav03::updateVal path: "<< path << ", resolved to: "<< npath <<", updated to value: '"<< val << "'" << std::endl;

	//	println(GenString()+RF("updated model from:")+getAsJson());
		// if val is a link, maybe reformat it to have no / at the end and ease search
		bool b= data.updateVal(npath,val);		// emit even when value was not modified ? even for links ?

	//	println(GenString()+RF("updated model to:")+getAsJson());
//		GenString check=data.get(npath);std::cout<< npath <<" written with '"<<check<<"'"<<std::endl;
		if(notifychange) emitIODataEvent(path,RF(WRITETAG));//delay events to avoid congesting event processing ?

		return b;
	}



	bool loadJSONContent(std::string jsoncontent){	// this function should be incorporated in GenTreeMap
		return loadJSONToGenTreeMap(jsoncontent,&data);
	};


	void addPlugIn(IODataPlugin *plugin){if(plugin) {plugins.push_back(plugin);plugin->setModel(this);}} // to remove when iodata is destroyed

	//	GenTreeMap *getDataTreeMap(){ return &data;} //not so nice for encapsulation

	GenString getAsJson(){return data.getAsJson();}


};



#endif
