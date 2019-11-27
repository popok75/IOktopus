#ifndef IODEVICEV01_H
#define IODEVICEV01_H
#include "../../infrastructure/CompatPrint.h"

#include "../../infrastructure/CompatIO.h"



#include "IODeviceGen.h"



//#include "../datamodel/IODataGen.h"
#undef FTEMPLATE
#define FTEMPLATE ".irom.text.iodevicev01"

#define PATH_KEYWORD "path"
#define DEFAULT_PATH "/nodes/"

class IODevicev01: public IODeviceGen
{
	IOReader *reader=0;
	GenMap *config;	// could use instead the same info stored in GenReader

public:
	virtual IOReader *getReader(){return reader;}
/*
	IODevicev01 (unsigned int num0,GenString name0, GenString modelname0, std::vector<unsigned int> pins0, GenMap subpaths0, GenString path0):IODeviceGen(num0, name0, modelname0,pins0,subpaths0,path0){
		println(RF("Constructor IODevicev01"));
		if(path[path.length()-1]=='/') path.erase(path.length()-1);	// can take /path/ or /path ending
	};
*/
	IODevicev01 (unsigned int num0, GenMap *globalconfig ){
			println(RF("Constructor IODevicev01 num :")+to_string(num0));
			config=globalconfig;
			num=num0;
	};




	void init(){
 		println("IODevicev01::init");
		GenString modelname=config->get(getFN(RF("model"),num));
		if(!reader && IOFactory::isSensor(modelname)) {
//			GenString pins=config->get(getFN(RF("pins")));
//			std::vector<unsigned int> pinvect=getPins(pins);
			GenString pins=config->get(getFN(RF("pins"),num));
			GenString model0=config->get(getFN(RF("model"),num));
			reader=IOFactory::createReader(model0,num,IOFactory::getPinsFromString(pins));
			if(!reader) return ;	//how to notify error
	//		println("IODevicev01::init2");

			reader->on(this);//&devicelistener);
//			println("IODevicev01::init3");
			reader->init();

			GenString autoreader=config->get(getFN(RF(AUTOREADER_KEYWORD),num));
			if(isDigit(autoreader)){
				reader->autoread((unsigned int)strToUnsignedLong(autoreader));
			}

//			reader->autoread(5);
			// we must capture value change and propagate it to model
		}
//		println("IODevicev01::init end");

	};

	GenString getPath(GenString path){
		for(auto& c : path) c = tolower(c);
		GenString str=config->get(getFN(GenString()+RF(PATH_KEYWORD)+RF("-")+path,num));
		if(str.empty()) str=config->get(getFN(RF(PATH_KEYWORD),num));
		return str;
	}

	bool notify(GenString ename,Event*event=0){
#ifdef ESP8266
#ifdef MEMDEBUG
		print(RF("IODevicev01:notify free memory :"));	println(ESP.getFreeHeap(),DEC);
#endif
#endif
		StringMapEvent *emap=0;
		if(event->getClassType()==StringMapEventTYPE) emap=(StringMapEvent*)(event);
		if(!emap || emap->values.empty()) return false; //pb

		for(auto it:emap->values) {
			GenString first=it.key();
			unsigned int i=first.find(RF("/"));
			if(i<first.size()){ // rename if there is a new name// append /nodes/ to path
				GenString sub=first.substr(0,i);
				GenString npath=getPath(sub);
				if(npath.empty()) continue;
				GenString nkey=GenString(RF(DEFAULT_PATH))+npath+first.substr(i);
				it.setKey(nkey);
//				println(nkey);
			}

		}

		println(GenString()+RF("IODevicev01::notify ")+emap->values.asJson());
		emit(RF(UPDATE_MODEL_EVENT),emap);	//path is name of event object while reading is the event listened to

		return true;
	};

private:
};



#endif
