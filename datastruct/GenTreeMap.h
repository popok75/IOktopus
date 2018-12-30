
/*
 Issues:
 	 - GenTreeMap, GenMap, GenString,  defines points to the implementation, not the interface (because we didn't want to make a factory)
 	 - no iterator provided for the treemap

 */


#ifndef GENTREEMAP_H
#define GENTREEMAP_H
// Prototype for a tree of strings (can store typeless data as well)

#include "GenString.h"
#include "GenMap.h"

class GenTreeMapProto
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
	virtual GenString get(GenString path)=0;

	virtual GenString getAsJson()=0;
};









#include <map>
#include <vector>
//#include <iostream> // for debug

class GenTreeMapStl: public GenTreeMapProto
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

	bool updateVal(GenString path, GenString valstring){
		if(path[0]=='/') path.erase(0,1);
		unsigned int index=path.find(RF("/")), pindex=0;
		GenTreeMapStlNode *cur=&root;
		while (pindex<(path.size()-1)) {
			GenString subkey=path.substr(pindex,index-pindex);
			GenTreeMapStlNode *ncur=cur->getNode(subkey);
			if(!ncur) {	//path not found
				ncur=cur->createKey(subkey);
			}
			cur=ncur;
			pindex=index+1;
			index=path.find(RF("/"),pindex);
			if (pindex>=(path.size()-1)){	// if no other /, e.g. /nodes/humidity/value, or last char is /, e.g. /nodes/humidity/value/
				cur->setContent(valstring);
				return true;
			}
			if (index>(path.size())) index= path.size();
		}

		return false;
	}

	GenString get(GenString path){
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

		return GenString();
	}

	GenString jsonEscape(GenString str){return (GenString)(RF("\"")+str+RF("\""));}	// simplified json escaping

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
			//	std::cout << jsonstr << std::endl;
				// if end go up, if final end stop
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

				if(cur->hasMap()){// if has children => // add its key+{, save node, and replace by sub node
					jsonstr+=RF("{");
					it++;
					vectit.push_back(it);
					it=cur->getMap()->begin();
					vectmap.push_back(cur->getMap());
					comma=false;
					continue;
				} else {	// no children, just add the value				// if no children => // add its key+value and go to next
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


#define GenTreeMap GenTreeMapStl

#endif
