
/*
 Issues:
 	 - GenTreeMap, GenMap, GenString,  defines points to the implementation, not the interface (because we didn't want to make a factory)
 	 - no iterator provided for the treemap
 	 - GenTreeMap and TreeMap keys are not ordered, order of creation is not preserved
 	 - use a spiffs like flat (ordered ?) genmap

 */


#ifndef GENTREEMAP_H
#define GENTREEMAP_H
// Prototype for a tree of strings (can store typeless data as well)

#include "GenString.h"
#include "GenMap.h"

class GenTreeMapProto	// GenTreeMapProto is assumed to have unique value per path, not like GenMap and multimaps
{
public:
	//	virtual ~GenTreeMapProto(){};

	bool update(GenString path, GenMap &valmap){	// update a subtree with key/value pairs
		bool ret=false;
		//		GenMap::Iterator it=valmap.begin();
		//		for(;!it.isEnd();(it)++){
		for(GenMap::Iterator it:valmap){
			GenString fullpath=path+RF("/")+it.key();
			bool b=updateVal(fullpath,it.value());
			if(b) ret=true;
		}

		return ret;
	}
	virtual bool updateVal(GenString path, GenString valstring)=0;
	virtual GenString get(GenString path)=0;	// return the value at the path,
	// or a null string the path does not exist or matches subkeys not a value

	virtual GenString findValuePath(GenString val, GenString startpath="")=0;

	virtual std::vector<GenString> listSubpaths(GenString path)=0;	// return a list of immediate subpath if not leaf

	virtual GenString findProgeny(GenString path,GenString startpath="")=0;	// return next leaf with same base=path


	virtual GenString getAsJson()=0;

	virtual bool hasPath(GenString path)=0;// if the value is "", the existance of the path cannot be verified with get(path), hence this function

};

GenString jsonEscape(GenString str){return GenString()+RF("\"")+str+RF("\"");}	// simplified json escaping







#include "GenMap.h"

// a simpler spiffs-like implementation with just one map and flat paths as keys
class GenTreeMapStlVect: public GenTreeMapProto
{
	GenMap datamap;
public:



	virtual bool updateVal(GenString path, GenString valstring){
		if(path.back()=='/') path.pop_back();
		//bool erased=
		datamap.erase(path);// erase (all) previous path-key if found : GenMap is multimap but GenTreeMapProto is not, so we remove previous value before writing new ones
		// if we remove previous lines, it would enable to store multiple values per path
		return datamap.set(path,valstring);
	};

	virtual GenString get(GenString path){// return the value at the path,// or a null string the path does not exist or matches subkeys not a value
		if(path.back()=='/') path.pop_back();
		return datamap.get(path);
	};
	bool hasPath(GenString path){
		if(path.back()=='/') path.pop_back();
		if(datamap.has(path)) return true;
		// must consider subpaths as well, e.g. if we have /a/b/c then we have /a and /a/b
		return !findProgeny(path).empty();
	}

	virtual GenString findProgeny(GenString pathbase,GenString startafterpath=""){
		bool comparenow=startafterpath.empty();
		for(GenMapProto::Iterator it:datamap){
			GenString k=it.key();
			if(comparenow && startsWith(k,pathbase)) return k;
			if(!comparenow && k==startafterpath) comparenow=true;
		}
		return "";
	}

	virtual GenString findValuePath(GenString val, GenString startpath=""){
		bool comparenow=startpath.empty();
		for(GenMapProto::Iterator it:datamap){
			GenString k=it.key();
			if(comparenow && it.value()==val) return k;
			if(!comparenow && k==startpath) comparenow=true;
		}
		return "";
	};

	virtual std::vector<GenString> listSubpaths(GenString path){	// return a list of the direct subkeys
		if(path.back()=='/') path.pop_back();
		std::vector<GenString> subkeys;
		GenMapProto::Iterator it2=datamap.begin();
		for(GenMapProto::Iterator it:datamap){
			GenString k=it.key();
			if(k!=path && startsWith(k,path)) {
				GenString subpath=k.substr(path.size());
				GenString subkey=getPathToken(subpath,0);
				bool add=true;
				for(GenString k : subkeys) if(subkey==k) {add=false;break;}	//already in, skip it
				if(add) subkeys.push_back(subkey);	// push only the key, not the full subpath
			}
		}
		return subkeys;
	};	// return a list of immediate subpath if not leaf


	GenString nextSiblingPath(GenString path){
		if(path.empty()) return "";
		GenString ppath=getPathParent(path);
		std::vector<GenString> sibvect=listSubpaths(ppath);
		GenString ckey=getPathLeaf(path);
		bool found=false;
		for(GenString sib:sibvect) {
			if(found) {
				if(ppath.back()=='/') ppath.pop_back();
				return ppath+'/'+sib+'/';}
			if(sib==ckey) found=true;
		}
		return "";
	}

	// should make an external JSON writer that require only tree traversal functions
	// could convert numeral only keys into array (if appropriate)
	GenString getAsJson() {		// a gentreemap::iterator allowing to go through each item would be useful and would allow a gen version of getasjson
		GenString jsonstr=RF("{");
		bool stop=false;
		if(!datamap.empty()){

			std::vector<GenString> keystack;
			GenMapProto::Iterator it=datamap.begin();	// we get the first item
			//	GenString ckey=getPathToken(it.key(),0);
			GenString cpath=GenString()+'/'+getPathToken(it.key(),0)+'/';

			bool comma=false, found=true;
			while(!stop){
				//		std::cout << jsonstr << std::endl;	// if end go up, if final end stop
				if(!found) {	// go up
					cpath=getPathParent(cpath);
					// stop
					if(cpath.size()<=1) {stop=true;continue;break;} //useless break... or continue
					// or go up
					GenString npath=nextSiblingPath(cpath);

					if(npath.empty()) found=false;
					else {
						cpath=npath;	// found next	//pb here when comparator
						found=true;
					}
					jsonstr+=RF("}");
					comma=true;
					continue;
				}
				if(comma) jsonstr+=RF(",");
				jsonstr+=jsonEscape(getPathLeaf(cpath))+RF(":");



				std::vector<GenString> vect=listSubpaths(cpath);
				if(vect.size()>0){// if has children => // add its key+{, save node, and replace by sub node
					jsonstr+=RF("{");
					//ckey=vect.front();
					cpath+=vect.front()+'/';
					comma=false;
					continue;
				} else {	// no children, just add the value				// if no children => // add its key+value and go to next
					jsonstr+=jsonEscape(get(cpath));
					comma=true;
				}

				//next item is the next subpath of parent
				GenString npath=nextSiblingPath(cpath);
				if(npath.empty()) found=false;
				else cpath=npath;

			}
		}
		jsonstr+=RF("}");
		return jsonstr;
	}

};








#include <functional>
#include <map>
#include <vector>
//#include <iostream> // for debug

class GenTreeMapStl: public GenTreeMapProto		// GenTreeMapStl deprecated in favor of GenTreeMapStlVect : shorter, faster and simpler
{
	class GenTreeMapStlNode;

	typedef std::map<GenString,GenTreeMapStlNode> StlMap;

	class GenTreeMapStlNode{
		GenString *content=0;
		StlMap* subtree=0;
	public:
		~GenTreeMapStlNode(){if(content) delete content;if(subtree) delete subtree;}

		GenTreeMapStlNode* getNode(GenString key){
			if(subtree) return &(*subtree)[key];
			return 0;
		};

		void setContent(GenString &ncontent){
			if(!content) content=new GenString(ncontent);
			else (*content)=ncontent;
		}	// copy string

		GenString getContent(){
			if(content) return *content;
			return GenString();
		}	// copy string


		GenTreeMapStlNode* createKey(GenString &nkey){
			if(!subtree) subtree=new StlMap();
			(*subtree)[nkey]=GenTreeMapStlNode();
			return &(*subtree)[nkey];
		}

		bool hasMap(){return subtree;}
		bool hasContent(){return subtree;}

		StlMap* getMap(){return subtree;}

	} root;

public:

	bool searchFunc(GenString path,GenString key, GenTreeMapStlNode * ){
		return true;
	}

	GenString buildPathFromTokens(std::vector<GenString> &pathtokens){
		GenString path=GenString()+'/';
		for(GenString k:pathtokens) path+=k+'/';
		return path;
	};

	virtual GenString findValuePath(GenString val, GenString startpath=""){

		if(startpath.empty() && !root.hasMap() && (!root.hasContent() || root.getContent()!=val)) return "";

		std::function<bool (GenString path,GenString key,GenTreeMapStlNode *item)> lambda
				= [&](GenString path,GenString key,GenTreeMapStlNode *item) {
			if(item->hasContent() && item->getContent()==val) return false;
			return true;
		};
		//lambda();
		return traversalPath(startpath, lambda);

	};


	GenString traversalPath(GenString startpath, std::function<bool (GenString,GenString,GenTreeMapStlNode *)> testfunc){ //bool(*testfunc)(GenString,GenString,GenTreeMapStlNode *)){
		// must go through all the tree and test each, stop if found false

		bool testfromnow=startpath.empty();// could be better optimized (skip other subdir traversal)
		std::vector<GenString> pathtokens;

		std::vector<StlMap::iterator> vectit;
		std::vector<StlMap*> vectmap;

		StlMap *curmap=root.getMap();
		vectmap.push_back(curmap);
		StlMap::iterator it=curmap->begin();
		bool stop=false;
		while(!stop){
			if(it==vectmap.back()->end()) {	//no more items
				// stop
				if(vectit.empty()) {stop=true;continue;break;} //useless break... or continue
				// or go up
				it=vectit.back();vectit.pop_back();
				vectmap.pop_back();
				pathtokens.pop_back();
				continue;
			}
			GenTreeMapStlNode *cur=&(it->second);
			GenString path=buildPathFromTokens(pathtokens)+it->first;
			if(testfromnow){
				bool b=testfunc(path,it->first,cur);
				if(!b) return path;
			}
			if(!startpath.empty() && startpath==path) testfromnow=true;
			if(cur->hasMap()){ 	// has children, we enter inside, go down
				pathtokens.push_back(it->first);	// save the name
				it++;
				vectit.push_back(it);
				it=cur->getMap()->begin();
				vectmap.push_back(cur->getMap());
				continue;
			}	// no children, we do nothing

			it++;
		}


		return "";
	}


	GenString pathItem(GenString path,unsigned int pos){	// return the token in the path at position pos, e.g. /dev/etc/shared,1-> etc
		return getPathToken(path,pos);
		/*		if(path.front()=='/') path.erase(0,1);	// remove first and last slash
		if(path.back()=='/') path.erase(path.size()-1,1);
		unsigned int index=path.find(RF("/")), pindex=0;
		GenString subkey=path.substr(pindex,index-pindex);
		while(pos>0) {
			if(index>=path.size()) return "";
			pindex=index+1;
			index=path.find(RF("/"),pindex);
			if(index>path.size()) index=path.size();
			subkey=path.substr(pindex,index-pindex);
			pos--;
		}
		return subkey;*/
	}


	bool updateVal(GenString path, GenString valstring){
		//		std::cout << "GenTreeMapStl::updateVal path:'"<<path<<"', value: '"<<valstring<<"'"<<std::endl;

		unsigned int i=0;
		std::string subkey=pathItem(path,i);
		GenTreeMapStlNode *cur=&root;
		while(!subkey.empty()){
			//			std::cout << "GenTreeMap::updateVal subkey:" << subkey << std::endl;
			GenTreeMapStlNode *ncur=cur->getNode(subkey);
			if(!ncur) ncur=cur->createKey(subkey);	//path not found, create it
			cur=ncur;
			i++;subkey=pathItem(path,i);
			if(subkey.empty()){ // last key in the path
				cur->setContent(valstring);
				return true;
			}

		}
		/*
		if(path[0]=='/') path.erase(0,1);	// remove first and last slash
		if(path[path.size()-1]=='/') path.erase(path.size()-1,1);

		unsigned int index=path.find(RF("/")), pindex=0;
		GenTreeMapStlNode *cur=&root;
		while (pindex<(path.size()-1)) {
			GenString subkey=path.substr(pindex,index-pindex);	//get the directory/file name
			std::cout << "GenTreeMap::updateVal subkey:" << subkey << std::endl;
			GenTreeMapStlNode *ncur=cur->getNode(subkey);
			if(!ncur) ncur=cur->createKey(subkey);	//path not found, create it

			cur=ncur;
			pindex=index+1;
			index=path.find(RF("/"),pindex);
			// bug : if no / at the end, not parsed ?
//			if (pindex>=(path.size()-1)){	// if path is 1 letter, this fail// if no other '/', e.g. /nodes/humidity/value, or last char is /, e.g. /nodes/humidity/value/
			if (pindex>=path.size())
			{
				cur->setContent(valstring);
				return true;
			}
			if (index>(path.size())) index= path.size();
		}
		 */
		return false;
	}

	std::vector<GenString> listSubpaths(GenString path){	// return a list of immediate subpath if not leaf

		unsigned int i=0;
		std::string subkey=pathItem(path,i);
		GenTreeMapStlNode *cur=&root;
		while(!subkey.empty()){
			//			std::cout << "GenTreeMap::updateVal subkey:" << subkey << std::endl;
			GenTreeMapStlNode *ncur=cur->getNode(subkey);
			if(!ncur) return std::vector<GenString>();//path not found
			cur=ncur;
			i++;subkey=pathItem(path,i);
			if(subkey.empty()){ // last key in the path
				std::vector<GenString> vect;
				if(!cur->getMap()) return vect;
				//	StlMap::iterator it=cur->getMap()->begin();
				for(StlMap::iterator it=cur->getMap()->begin();it!=cur->getMap()->end();it++){
					//GenTreeMapStlNode *cur=&(it->second);
					vect.push_back(it->first);
				}
				return vect;
			}

		}

		/*
		if(path[0]=='/') path.erase(0,1);
		unsigned int index=path.find(RF("/")), pindex=0;
		GenTreeMapStlNode *cur=&root;

		while (pindex<(path.size()-1)) {
			GenString subkey=path.substr(pindex,index-pindex);
			cur=cur->getNode(subkey);
			if(!cur) return std::vector<GenString>();//path not found
			pindex=index+1;
			index=path.find('/',pindex);
			if (pindex>=(path.size()-1)){	// if no other /, e.g. /nodes/humidity/value, or last char is /, e.g. /nodes/humidity/value/
				std::vector<GenString> vect;
				if(!cur->getMap()) return vect;
			//	StlMap::iterator it=cur->getMap()->begin();
				for(StlMap::iterator it=cur->getMap()->begin();it!=cur->getMap()->end();it++){
					//GenTreeMapStlNode *cur=&(it->second);
					vect.push_back(it->first);
				}
				return vect;

			}
			if (index>(path.size())) index= path.size();
		}
		 */
		return std::vector<GenString>();
	};

	GenString get(GenString path){

		unsigned int i=0;
		std::string subkey=pathItem(path,i);
		GenTreeMapStlNode *cur=&root;
		while(!subkey.empty()){
			//			std::cout << "GenTreeMap::get subkey:" << subkey << std::endl;
			GenTreeMapStlNode *ncur=cur->getNode(subkey);
			if(!ncur) return GenString();	//path not found,
			cur=ncur;
			i++;subkey=pathItem(path,i);
			if(subkey.empty()) return cur->getContent(); // last key in the path
		}

		/*
		if(path[0]=='/') path.erase(0,1);
		unsigned int index=path.find(RF("/")), pindex=0;
		GenTreeMapStlNode *cur=&root;
		while (pindex<(path.size()-1)) {
			GenString subkey=path.substr(pindex,index-pindex);
			cur=cur->getNode(subkey);
			if(!cur) {	//path not found
				return GenString();
			}
			pindex=index+1;
			index=path.find('/',pindex);
			if (pindex>=(path.size()-1)){	// if no other /, e.g. /nodes/humidity/value, or last char is /, e.g. /nodes/humidity/value/
				return cur->getContent();
			}
			if (index>(path.size())) index= path.size();
		}
		 */
		return GenString();
	}



	// could convert numeral only keys into array (if appropriate)
	GenString getAsJson() {		// a gentreemap::iterator allowing to go through each item would be useful and would allow a gen version of getasjson
		GenString jsonstr=RF("{");
		bool stop=false;
		if(root.hasMap()){
			std::vector<StlMap::iterator> vectit;
			std::vector<StlMap*> vectmap;
			StlMap::iterator it=root.getMap()->begin();
			bool comma=false;
			vectmap.push_back(root.getMap());
			while(!stop){
				//		std::cout << jsonstr << std::endl;	// if end go up, if final end stop
				if(it==vectmap.back()->end()) {
					// stop
					if(vectit.empty()) {stop=true;continue;break;} //useless break... or continue
					// or go up
					it=vectit.back();vectit.pop_back();
					vectmap.pop_back();
					jsonstr+=RF("}");
					comma=true;
					continue;
				}

				GenTreeMapStlNode *cur=&(it->second);

				if(comma) jsonstr+=RF(",");
				jsonstr+=jsonEscape(it->first)+RF(":");
				/*			std::string addition=jsonEscape(it->first)+RF(":");
				if(addition.find("dst[1]")<addition.size()) {
					std::cout << addition << std::endl;
				}*/

				if(cur->hasMap()){// if has children => // add its key+{, save node, and replace by sub node
					jsonstr+=RF("{");
					it++;
					vectit.push_back(it);
					it=cur->getMap()->begin();
					vectmap.push_back(cur->getMap());
					comma=false;
					continue;
				} else {	// no children, just add the value				// if no children => // add its key+value and go to next
					//	std::string addition2=jsonEscape(cur->getContent()), addition3=cur->getContent();
					jsonstr+=jsonEscape(cur->getContent());
					comma=true;
				}
				it++;
			}
		}
		jsonstr+=RF("}");
		return jsonstr;
	}

};


#define GenTreeMap GenTreeMapStlVect

#endif
