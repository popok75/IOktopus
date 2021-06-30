#ifndef GENMAP_H
#define GENMAP_H

#include <initializer_list>

#include "GenString.H"

//#define FF(string_literal) (reinterpret_cast<const __FlashStringHelper *>(((__extension__({static const char __c[] __attribute__((section(".irom.text.template"))) = ((string_literal)); &__c[0];})))))
//#define RF(x) String(F(x)).c_str()
#undef FTEMPLATE
#define FTEMPLATE ".irom.text.genmap"


//#include "../infrastructure/compatPrint.h"

/*
 Issues:
 	 - GenTreeMap, GenMap, GenString,  defines points to the implementation, not the interface (because we didn't want to make a factory)
 	 - iterator need to be deleted after use : not possible to return covariant derived class by value
		-> use subobject derivation like in genobjmap
 */


template <typename T> std::string intoString(T ptr);	// store any type in a string
template <typename T> T intoType(std::string strval);	//restore the type (limited to 64bit max)

#include <map>

// Prototype for Tree of Strings
//static

// static GenString voidstring;	// how to avoid having this string in the sram -> moved to dram see l30


class GenMapProto {

public:
	GenMapProto(){};
	GenMapProto(std::map<GenString,GenString> &src){
		for(std::map<GenString,GenString>::iterator iter = src.begin(); iter != src.end(); ++iter)
			set(iter->first,iter->second);
	};
	GenMapProto(std::multimap<GenString,GenString> &src){
		for(std::map<GenString,GenString>::iterator iter = src.begin(); iter != src.end(); ++iter)
			set(iter->first,iter->second);
	};
	virtual ~GenMapProto(){};

	class Iterator;
	class IteratorConnector {
	public:
		virtual ~IteratorConnector(){};
		virtual bool goToNext(Iterator *it)=0;// return true if list it has changed position
		virtual IteratorConnector* duplicate(Iterator *it)=0;
		virtual bool commit(){return true;};
		virtual bool update(){return true;};
		virtual void setKey(GenString newkey)=0;
		virtual void setValue(GenString newval)=0;
		virtual void set(GenString newkey, GenString newval)=0;
	};

	class Iterator {
		GenString voidstring;	// how to avoid having this string allocated at each object ?  what reference to return for key null ?
	public:
		unsigned int index=0;

		IteratorConnector*conn=0;
	 	GenString *itkey=0,*itval=0;

		Iterator(IteratorConnector*conn0=0,GenString *key0=0,GenString *val0=0,unsigned int index0=0):
			index(index0),conn(conn0),itkey(key0),itval(val0){voidstring.reserve(0);voidstring.shrink_to_fit();};
		virtual ~Iterator(){if(conn) delete conn;};
		Iterator(const Iterator& it)
		{
			index=it.index;
			itkey=it.itkey;itval=it.itval;
			conn=it.conn->duplicate(this);
		}
		Iterator& operator * ()	{return *this;}
		bool operator!= (Iterator &it2) const {return !((*this)==it2);}
		bool operator== (Iterator &it2) const {
			if((this->itkey==0)	&& (it2.itkey==0)) return true; //if voided no need to compare val
			if((this->itkey==0)	|| (it2.itkey==0)) return false;
			return ((itkey==it2.itkey) && (itval==it2.itval));
		}
		Iterator& operator++ (){     // prefix ++
			index++;
			if(conn) conn->goToNext(this);
			return *this;
		}

		Iterator& operator++ (int){     // postfix ++
			index++;
			if(conn) conn->goToNext(this);
			return *this;
		}

		virtual GenString *keyptr(){return itkey;};
		virtual GenString key(){
				if(itkey==0) {voidstring.resize(0);return voidstring;}
				return *itkey;
				};
		virtual GenString keyReference(){
			//if(conn) conn->key();
			//	println(GenString()+"GenMap : &key");
			if(itkey==0) {voidstring.resize(0);return voidstring;}
			return *itkey;
		};
		virtual GenString value(){if(itval==0) return voidstring;return *itval;}
		virtual GenString *valueptr(){return itval;};
		virtual GenString valueReference(){if(itval==0) return voidstring;return *itval;};

		virtual bool isEnd(){return (itkey==0);}

		virtual bool setValue(GenString nval){conn->setValue(nval);return true;}
		virtual bool setKey(GenString nkey){conn->setKey(nkey);return true;}
		virtual bool set(GenString nkey,GenString nval){conn->set(nkey,nval);return true;}

		virtual bool commit(){if(conn) return conn->commit(); else return false;};
		virtual bool update(){if(conn) return conn->update(); else return false;};
	};

	virtual Iterator begin()=0;//{return Iterator();};
	virtual Iterator end(){return Iterator();};

	virtual bool set(GenString key,GenString value){return false;} //return true if modified (key inexistant or with different value)
	virtual bool setReplace(GenString key,GenString value){return false;} // replace the key is existant

	void initFromBraced( std::initializer_list<std::initializer_list<GenString>> init_list){			//	std::map<GenString,GenString> mapval=map;
		for(std::initializer_list<GenString> il : init_list){
			int i=0;
			GenString key,value;
			for(GenString gs :il) {
				if(i==0) key=gs;
				if(i==1) {value=gs;break;}
				i++;
			}
			set(key,value);
		}
	};

	virtual GenString asJson(){
		GenString str=RF("{");
		bool first=true;
		for(auto it:*this){
			if(first) first=false;
			else str+=RF(",");
			str+=RF("\"")+it.key()+RF("\":\"")+it.value()+RF("\"");
		}
		str+=RF("}");
		return str;
	}


	virtual unsigned int size()=0;

	virtual bool empty()=0;//{return true;}

	virtual GenString get(GenString key){return GenString();};	// return value associated with key
	inline GenString operator[] (GenString key) {return get(key);}

	virtual bool has(GenString key)=0;//{return false};

	template <typename T> bool setObj(GenString key, T value){
		std::string strval=intoString<T>(value);
		return set(key,strval);
	};
	template <typename T> T getObj(GenString key){
		std::string strval=get(key);
		return intoType<T>(strval);
	};

};


template <typename T> std::string intoString(T ptr){	// store any type in a string
	return std::string((char *)&ptr,sizeof(ptr));
};
template <typename T> T intoType(std::string strval){	//restore the type (limited to 64bit max)
	uint64_t result=0,f=1;
	for(unsigned char c :strval){
		result+=f*c;
		f*=0x100;
	}
	return (T)result;
};








//#define AUTOSHRINK 1

#include <map>

class GenMapStl : public GenMapProto {
	std::multimap<GenString,GenString> stlmap;
	//StlIterator iter;
public:

	class StlIteratorConnector : public IteratorConnector {
		std::multimap<GenString,GenString> *stlmap;
		std::multimap<GenString,GenString>::iterator stlit;
	public:
		StlIteratorConnector(std::multimap<GenString,GenString> *stlmap0=0):stlmap(stlmap0){if(stlmap) stlit=stlmap->begin();}
		StlIteratorConnector(const StlIteratorConnector& it){stlmap=it.stlmap;if(stlmap) stlit=stlmap->begin();}
		virtual ~StlIteratorConnector(){};
		GenString *keyptr(){return (GenString *)&(stlit->first);};
		GenString *valueptr(){return (GenString *)&(stlit->second);};

		StlIteratorConnector* duplicate(Iterator *){return new StlIteratorConnector(stlmap);}

		void setKey(GenString newkey){GenString *k=keyptr();*k=newkey;	}
		void setValue(GenString newval){stlit->second=newval;}
		void set(GenString newkey, GenString newval){GenString *k=keyptr();*k=newkey;stlit->second=newval;}

		void init(Iterator *it){
			if(it){
				it->index=0;
				it->itkey=keyptr();
				it->itval=valueptr();
			}}
		bool isEnd(){
			return ((stlit)==stlmap->end());
		}
		bool goToNext(Iterator *it){
			if(isEnd()) return false;
			stlit++;
			if(isEnd()) it->itkey=0;
			else {
				it->itkey=keyptr();
				it->itval=valueptr();
			}
			return true;}

	};
	GenMapStl():GenMapProto(){}
	GenMapStl( std::initializer_list<std::initializer_list<GenString>> init_list){//:iter(&stlmap){
		initFromBraced(init_list);
	};

	GenMapStl(std::multimap<GenString,GenString> &src): GenMapProto(){
		for(auto it:src) {
			set(it.first,it.second);
		}
	}

	//using GenMapProto::GenMapProto;

	//	bool empty(){return stlmap.empty();}
	unsigned int size(){return stlmap.size();}

	bool set(GenString key,GenString value){
		std::multimap<GenString,GenString>::iterator it=stlmap.find( key );
		bool b=it != stlmap.end();	// search if we have a prior key
		if(b && it->second==value)	return false;
		//key.shrink_to_fit();value.shrink_to_fit();// probably wont be modified for a while
		if(!b) stlmap.insert(std::make_pair(key,value));
		else it->second=value;
		//	if(AUTOSHRINK) for (auto it:*this){it.itkey->shrink_to_fit();it.itval->shrink_to_fit();}
		return true;
	};

	virtual bool empty(){return stlmap.empty();}

	Iterator begin(){
		//		println(GenString()+"GenMapStl : begin s:"+to_string(stlmap.size()));
		if(stlmap.empty()) return end();
		StlIteratorConnector *conn=new StlIteratorConnector(&stlmap);
		Iterator it= Iterator(conn);
		conn->init(&it);
		return it;
	};
	GenString get(GenString key){
		auto it=stlmap.find( key );
		if(it == stlmap.end())	return GenString();
		return it->second;};
	bool has(GenString key){return !(stlmap.find(key) == stlmap.end());};

};
/*
class GenMapStlVector2 : public GenMapProto {
	// we use pair for key, impair for values
	std::vector<GenString> keynvalues;

};
 */

class GenMapStlVector : public GenMapProto {	// another idea to implement based on stl but slower but more memory efficient than multimap
	// map stored as pairs in a vector
	struct StringPair{
		GenString key,value;
		StringPair(GenString key0, GenString value0): key(key0), value(value0){};
	};

	std::vector<StringPair> values;

};




/* this is outdated ? remove ?
class GenMapObj: public GenMapStl {

	public:
	GenMapObj():GenMap(){};

	bool set(GenString key, GenString value){	// return true if found
		return setStringValue(key,value);
	};
	GenString get(GenString key){
		return getStringValue(key);
	};


	//	template <typename Tval> inline Tval operator [](StringType str)  {return get<Tval>(str);}	// allow reading [] but no writing
};
 */


#include "SSMap.h"


class SSString : public GenString {
	SSString(const SSString& str):GenString(str){}	//overload copy constructor possible ?
};


class GenSSMap : public GenMapProto, SSMap {	// the bigger the string supporting the map, the slower it will become,
													// maybe should be a version split in blocks of limited size ?
												// in addition stringtype used by ssmap can be subjected to size limitation
public:
	GenSSMap():GenMapProto(), SSMap(){multimap=false;}
	GenSSMap( std::initializer_list<std::initializer_list<GenString>> init_list):GenMapProto(), SSMap(){multimap=false;
		initFromBraced(init_list);
	};

	GenSSMap(GenSSMap &src): GenMapProto(), SSMap(src) {multimap=false;}
	GenSSMap(const GenSSMap &src): GenMapProto(), SSMap(src) {multimap=false;};//keys=src.keys; data=src.data;}

	GenSSMap(std::multimap<GenString,GenString> &src): GenMapProto(), SSMap() {multimap=false;
		for(auto it:src) {
			_set(it.first,it.second);
		}
	}
	unsigned int size(){return keys;}

	bool set(GenString key,GenString value){
		return _set(key,value);
	};

	bool setReplace(GenString key,GenString value){
		if(has(key)) {
			GenString v=get(key);
			return replace(key,v,key,value);
		}
		else return set(key,value);
	}
	virtual bool empty(){return (keys==0);}

	bool erase(GenString key){
		return SSMap::erase(key);
	}

	Iterator begin(){
		//		println(GenString()+"GenMapStl : begin s:"+to_string(stlmap.size()));
		if(empty()) return end();
		SSIteratorConnector *conn=new SSIteratorConnector(this);
		Iterator it= Iterator(conn);
		conn->init(&it);
		return it;
	};
	GenString get(GenString key){
		unsigned int index=0;
		GenString str=this->getNextValueForKey(key,index);
		return str;
	};
	bool has(GenString key){	return hasKey(key);}
		//unsigned int index=0;return !this->getNextValueForKey(key,index).empty();}; //this doesnt work for empty values

	class SSIteratorConnector : public IteratorConnector {
			unsigned int index=0;
			GenSSMap *ssmap;
			GenString keycopy, valcopy;	//extracted from the map
			GenString keyorig, valorig;	//extracted from the map
		public:
			SSIteratorConnector(GenSSMap *ssmap0=0):ssmap(ssmap0){}
			SSIteratorConnector(const SSIteratorConnector& it){ssmap=it.ssmap;index=it.index;keycopy=it.keycopy;valcopy=it.valcopy;keyorig=it.keyorig;valorig=it.valorig;}
			virtual ~SSIteratorConnector(){};
			GenString *keyptr(){return (GenString *)&(keycopy);};	//no way to get a pointer (it is the middle of a string)
			GenString *valueptr(){return (GenString *)&(valcopy);};

	//		SSIteratorConnector* duplicate(){return new SSIteratorConnector(*this);}
			SSIteratorConnector* duplicate(Iterator *it){
				SSIteratorConnector*ssit=new SSIteratorConnector(*this);
				it->itkey=&(ssit->keycopy);
				it->itval=&(ssit->valcopy);
				return ssit;
			}

			bool commit(){// inplace modification is possible in a ssmap, leaving the iterator position unchanged
				if(keycopy!=keyorig || valcopy!=valorig) {ssmap->replace(index,keycopy,valcopy);return true;}
				return false;
			};

			bool update(){	// read again from the ssmap
				keyorig=keycopy=ssmap->key(index);
				valorig=valcopy=ssmap->value(ssmap->nextValue(index));
				return true;
			};

			void setKey(GenString newkey){ssmap->replace(index,newkey,valcopy);}
			void setValue(GenString newval){ssmap->replace(index,keycopy,newval);}
			void set(GenString newkey, GenString newval){ssmap->replace(index,newkey,newval);}

			void init(Iterator *it){
				if(it){
					it->index=0;
					index=ssmap->nextKey(0);;
					update();
					it->itkey=&keycopy;
					it->itval=&valcopy;
				}
			}

			bool isEnd(){
				return (index>=(ssmap->getDataLength()-1));
			}
			bool goToNext(Iterator *it){
				commit();
				if(isEnd()) {
					it->itkey=0;
					return false;
				}
				else {
					index=ssmap->nextKey(index);
					if(isEnd()) it->itkey=0;
					update();
				}
				return true;
			}


		};
};


#define GenFastMap GenMapStl	// faster but less memory efficient (strings stored separately)
#define GenMap GenSSMap	// slower but more memory efficient (all strings stored in one string)
//#define GenObjMap GenObjMapProtoStlVector

#endif


/*
// map/initializer_list : 0 bytes SRAM
#include <initializer_list>
#include <map>
void setup() {
  std::initializer_list<std::initializer_list<int>> i;
  std::multimap<int,int> stlmap;
  std::map<int,int> src;
  int n;
 stlmap.find( n );
 stlmap.end();
 stlmap.insert(std::make_pair(n,n));
 stlmap.empty();
 std::multimap<int,int>::iterator it =stlmap.begin();
}
void loop() { }

 */
