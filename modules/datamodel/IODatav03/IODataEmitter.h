#ifndef IODATAEMITTER_H
#define IODATAEMITTER_H

//#include "../../../infrastructure/events/DefaultEventEmitter.h"
#include "../IODataGen.h"


/*	This file encapsulate Links and Events in IODataEmitter
 *
 *  Links : If the string data start with '/' character, it is considered a link. When reading or writing it will be the target that will be affected.
 * 			E.g. if "/nodes/a/val"="/nodes/b/val" and we write "1" in "/nodes/a/val", it will be "/nodes/b/val" that will be written at "1"
 *			note : '#' character at the end of path, e.g. '/nodes/a#', allows to override the link and modify the original path value
 * 	Events: Each write will trigger notification events to the listeners registered with the following paths :
 * 			1. The initial path modified, and all the link chain until final target,
 * 			2. All the paths with a value that link (absolute or relative) to the paths in 1.
 * 			3. All the parents paths of the paths from 1. and 2. (including the root for each write)
 */



#define LINKCHAR '#'
#define SEPCHAR '#'

//////////////////////////////////////////// DATALINKER class
class DataLinker {	// manage values as links

public:
	virtual GenString getDataVal(GenString path)=0;	// get direct access to data value (no links)

protected:
	std::vector<GenString> getLinkChain(GenString &path){
		GenString val=getDataVal(path);
		std::vector<GenString> vect;
		vect.push_back(path);
		while(getPathIsLink(val)) {
			path=getPathAbs(path,val);
			for(auto s : vect) if(s==path) return vect;	//if circular, return the last one
			vect.push_back(path);
			val=getDataVal(path);
		}
		return vect;
	}

	// method for modificiation without link follow : # after path
	bool isNoFollowLink(GenString &path){return path.back()==LINKCHAR;}; //if contain # as last character then, dont follow the link chain
	GenString getNoFollowLink(GenString path){
		if(!isNoFollowLink(path)) return path;
		path.pop_back();return path;};

	GenString resolveLink(GenString path){
		if(isNoFollowLink(path)) return getNoFollowLink(path);	// if no follow, return the link without #
		return getLinkChain(path).back();
	}

};

////////////////////////////////////////////// IODATALISTENER

// listener that adds a tag+path
class IODataListener : public EventListener {
	bool busy=false;
public:

	virtual bool notify(GenString path,GenString  tag, Event*event=0)=0;

	virtual bool notify(GenString ename,Event*event=0){	// parse tag, could convert to a IODataEvent* (genmap event)
		if(busy) return true;
		busy=true;	// prevent calling itself, if the listener trigger another event during his notify, it will be dropped
		unsigned int i=ename.find(SEPCHAR);
		if(i<ename.size()) {
			GenString path=ename.substr(0,i);
			GenString tag=ename.substr(i+1);
			bool r= notify(path,tag,event);
			busy=false;
			return r;
		} else {
			bool r= notify(ename,"",event);
			busy=false;
			return r;
		}
	};



};



////////////////////////////////////////////// IODATAEMITTER class
class IODataEmitter : public IODataGen, public DataLinker {

public:
	virtual void on(GenString path, GenString tag, EventListener*e){
		//	std::cout << "IODataEmitter::on path :" << path<< ", tag :" << tag << std::endl;
		DefaultEventEmitter::on(path+SEPCHAR+tag,e);
	};	// subscribe
	virtual void once(GenString path, GenString tag, EventListener*e){DefaultEventEmitter::once(path+SEPCHAR+tag,e);};	// subscribe to next event only
	virtual void removeListener(GenString path, GenString tag, EventListener*e){DefaultEventEmitter::removeListener(path+SEPCHAR+tag,e);};	// unsubscribe
	virtual GenString searchValue(GenString path, GenString startpath="")=0;

	virtual bool emit2(GenString path, GenString tag,  Event*event=0){
		//	std::cout << "IODataEmitter::emit path :" << path<< ", tag :" << tag << std::endl;
		return DefaultEventEmitter::emit(path+SEPCHAR+tag,event);};// Returns true if the event had listeners, false otherwise.
	// if we move to async we should use event instead *event ?


protected:
	bool emitIODataEvent(GenString path, GenString tag){

		if(isNoFollowLink(path)) return emit(getNoFollowLink(path)+SEPCHAR+tag);		// if path provided contain '#' as last char
		// if node is change we still need backward links // applies a.Always rule

		std::vector<GenString> paths=getLinkChain(path);	// get forward links
		//for(GenString s : paths) emitHierarchicLink(path,RF("write"));	// emit event on each and their backward links, and recursively
		unsigned int index=0;
		while(index<paths.size()){
			GenString path=paths[index];
			addBackLinksTo(path,paths);
			addBackRelLinksTo(path,paths);
			addHierarchicParentsTo(path,paths);
			index++;
		}
		// maybe should order paths and
		unsigned int tok=0;
		for(GenString str : paths) {
			unsigned n=getPathTokenNumber(str);
			if(n>tok) tok=n;
		}
		for(;tok>0;tok--){ //start with most specific, and let the less token for the end ? by decreasing token number ?
			for(GenString str : paths) {
				if(getPathTokenNumber(str)==tok) emit(str+SEPCHAR+tag);
			}
		}

		//	std::cout << "emit from path "<< path <<std::endl;
		/*		for(GenString str : paths) {
		//		std::cout << "propagate to path "<< str <<std::endl;
				emit(str,tag);
			}
		 */
		return true;
	}
private:
	// write event emit current rules: when a path is asked to be updated,
	// a. Always: this path will receive an write event (even if not modified)
	// b. Forward: if contains a link to a target path, the target is also notified, and recursively (no cycles allowed)
	// d. Hierarchic: each final event will be sent for all the hierarchic levels of the path, including root, e.g. /user/pok/file, /user/pok/, /user/, /
	// c. Backward: if any node contain this path as link, emit as well, and recursively

	// A lot searches
	// storing separately the links would allow to save search time, at the expense of memory use
	// other options to optimize searches ?

	bool linkInList(GenString link, std::vector<GenString> &list){
		for(GenString str:list) if(link==str) return true;
		return false;
	}

	void addBackLinksTo(GenString path, std::vector<GenString> &paths){
		GenString fpath=searchValue(path);	 // find the abs path in model
		while(!fpath.empty()){
			if(!startsWith(fpath,path)
					&& !linkInList(fpath,paths))//check that it is notpointing to his own parent, i.e. fpath does not contain path
				paths.push_back(fpath);	//recursively emit event to the items pointing on fpath	// applies c.Backward rules
			fpath=searchValue(path,fpath);
		}
	}

	void addBackRelLinksTo(GenString path, std::vector<GenString> &paths){
		GenString ppath=getPathParent(path);// try with relative path as well, e.g. /aa/bb/cc/dd -> ./dd , ./cc/dd , ./bb/cc/dd , ./aa/bb/cc/dd
		while(ppath.size()>1){
			GenString rpath=path.substr(ppath.size());
			rpath=GenString()+'.'+rpath;
			GenString fpath=searchValue(rpath);	 // find the abs path in model
			while(!fpath.empty()){
				if(startsWith(fpath,ppath) &&
						!startsWith(fpath,path) &&
						!linkInList(fpath,paths)) 	//check that it is notpointing to his own parent, i.e. fpath does not contain path
					paths.push_back(fpath);	//recursively emit event to the items pointing on fpath	// applies c.Backward rules
				fpath=searchValue(path,fpath);
			}
			ppath=getPathParent(ppath);
		}
	}

	void addHierarchicParentsTo(GenString path, std::vector<GenString> &paths){
		path=getPathParent(path);
		while(path.size()>1){
			if(!linkInList(path,paths)) paths.push_back(path);
			path=getPathParent(path);
		}
	};


};











#include "../../../datastruct/GenTreeMap.h"
#include "../../../datastruct/JSON.h"
// adds a path listening




class GenTreeMapHeavyJSONLoader: public HeavyJSONLoader {
	GenTreeMap *map=0;
	GenString prepath;
public:
	GenTreeMapHeavyJSONLoader (GenTreeMap *map0, std::string *json=0, bool cleanjson0=false, std::string *err0=0):HeavyJSONLoader(json,cleanjson0,err0), map(map0){};
	void setPrepath(GenString prepath1){prepath=prepath1;}

	virtual bool valueCallBack(){
		GenString path=getPath()+stringKey();
		//std::cout << "MyHeavyJSONLoader:"<<prepath<<path << ":" << stringValue() << std::endl;
		if(map) map->updateVal(prepath+path, stringValue());
		return JSONLoader::valueCallBack();
	};
};

bool loadJSONToGenTreeMap(GenString &jsoncontent, GenTreeMap *treemap, GenString prepath=""){
	if(!checkSyntaxJSON(jsoncontent)) return false;
	GenTreeMapHeavyJSONLoader myheavyloader(treemap);
	if(!prepath.empty()) myheavyloader.setPrepath(prepath);
	parseJSON(jsoncontent,&myheavyloader);
	return true;
};











#endif
