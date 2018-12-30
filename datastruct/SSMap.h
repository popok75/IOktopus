#ifndef SSBASE_H
#define SSBASE_H

//#include "GenMap.h"

#ifndef StringType
#include <string>
#define StringType std::string
#endif


// replace once in same string all occurrence of pattern by newval from start to end index
inline bool smcreplaceAllInplace(std::string &src, std::string pattern,std::string newval,std::string::size_type start, std::string::size_type end){
	bool changed=false;
	while ( ( start = src.find( pattern, start ) ) < end )
	{
		src.replace( start, pattern.size(), newval );
		changed=true;
		start += newval.size();
	}
	return changed;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// StringMap : fast and efficient string map using light encoding feature
//	- key-val association
//  - option of unique keys,vs keys/value pairs
/*
char smcintrapair='\t';
char smcinterpair='\n';
StringType smcintrapairunescaped=RF("\t"),smcintrapairescaped=RF("\\t");
StringType smcinterpairunescaped=RF("\n"),smcinterpairescaped=RF("\\n");
*/
#define smcintrapair '\t'
#define smcinterpair '\n'
#define smcintrapairunescaped GenString(RF("\t"))
#define smcintrapairescaped GenString(RF("\\t"))
#define smcinterpairunescaped GenString(RF("\n"))
#define smcinterpairescaped GenString(RF("\\n"))

class SSMap {
protected:
	StringType data;
	unsigned int keys=0;
public:
	bool fastescapekey=true;	//if keys and values are garanteed to not contain escape char they can not be escaped
	bool fastescapevalue=true;
	bool multimap=true;	//if it is true, a set for a key do not remove previous entry, is only canceled when value is also already there

	SSMap(){data=StringType()+smcinterpair;};
	SSMap(bool uniquekeys0){data=StringType()+smcinterpair;};

	SSMap(SSMap &original){keys=original.size(); data=original.data;}//copy
	SSMap(const SSMap &original){keys=original.keys; data=original.data;}//copy

	inline unsigned int size(){return keys;}
	inline unsigned int getDataLength(){return data.size();}
	inline bool empty(){return (keys==0);};
	//	inline unsigned int size(){return keys;}
	//	inline unsigned int getByteSize(){return data.size();}
	//	inline StringType *getData(){return &data;}

	inline StringType key(unsigned int i){// return key at pos i or "" if not a key
		if(i!=0 && data[i-1]!=smcinterpair) return "";
		unsigned int j=data.find(smcintrapair,i);
		return data.substr(i,j-i);
	};
	inline StringType value(unsigned int i){
		//		StringType strdebug=data.substr(i,10);
		if(data[i-1]!=smcintrapair) return "";
		unsigned int j=data.find(smcinterpair,i);
		return data.substr(i,j-i);
	}

	bool _set(StringType key, StringType value){	//set the key to value, add it if not found, // return true if changed the map
		if(!fastescapekey) escape(key);
		if(!fastescapevalue) escape(value);

		if(hasKeyValueEscaped(key,value)) return false;

		if(!multimap){
			unsigned int i=nextKey(key),ds=data.size();
			if(i<ds){	// update content of key
					unsigned j=nextKey(i+1)+key.size()+1; // ??? debug here
					data.erase(i,j-i);
					data.insert(i,value);
					return true;
			}
		}
		data+=key+smcintrapair+value+smcinterpair;
		keys++;
		return true;
	}

	bool hasKeyValueEscaped(StringType &key, StringType &value){
		if(!multimap) return get(key)==value;
		unsigned int i=nextKey(key),ds=data.size();
		bool b=true;
		while(i<ds){
			if(b) {i+=key.size()+1;b=false;}
			else i=nextValue(i);
			unsigned j=nextKey(i+1)-1;
			if(j>ds) j=ds-1;
			StringType val=data.substr(i,j-i);
			if(val==value) return true;
			i=nextKey(key,j);
		}
		return false;
	}
	bool hasKeyValue(StringType &key, StringType &value){
		if(!fastescapekey) escape(key);
		if(!fastescapevalue) escape(value);

		return hasKeyValueEscaped(key,value);
	}


	virtual inline bool replaceValueAt(unsigned int pos, StringType newvalue){
		if(!fastescapevalue) escape(newvalue);
		unsigned i=data.find(smcinterpair,pos);
		data.erase(pos,i-pos);
		data.insert(pos,newvalue);
		return true;
	};

	StringType get(StringType key){
		if(!fastescapekey) escape(key);
		unsigned int i=nextKey(key);
		if(i<data.size()) {
			i+=key.size()+1;
			unsigned int j=nextKey(i+1)-1;
			StringType value= data.substr(i,j-i);
			if(!fastescapevalue) unescape(value);
			return value;
		}
		else return "";
	};	// get the first value matching the key

	bool erase(StringType key){
		bool ret=false;
		if(!fastescapekey) escape(key);
		unsigned int i=nextKey(key);
		while(i<data.size()) {
			i+=key.size()+1;
			unsigned int j=nextKey(i+1);
			data.erase(i,j-i);
			keys--;
			i=nextKey(key,i);
			ret=true;
			if(!multimap) break;
		}
		return ret;
	}

	bool erase(StringType key, StringType value){	// there is no erase taking place here ??!!??
	//	bool ret=false;
		if(!fastescapekey) escape(key);
		if(!fastescapekey) escape(value);

		unsigned int i=data.find(smcinterpair+key+smcintrapair+value+smcinterpair);
		if(i<data.size()) {
			data.erase(i,key.size()+value.size()+2);
			return true;
		}
		else return false;
	}

	bool replace(unsigned int i,StringType nkey, StringType nvalue){
	//	bool ret=false;
		if(!fastescapekey) {escape(nkey);}
		if(!fastescapevalue) {escape(nvalue);}

		if(i<data.size()){
			unsigned int j=this->nextKey(i);
			//data.erase(0,21);
			data.replace(data.begin()+i,data.begin()+j,nkey+smcintrapair+nvalue+smcinterpair);
			return true;
		}
		else return false;
	}

	bool replace(StringType key, StringType value,StringType nkey, StringType nvalue){
	//	bool ret=false;
		if(!fastescapekey) {escape(key);escape(nkey);}
		if(!fastescapevalue) {escape(value);escape(nvalue);}

		unsigned int i=data.find(smcinterpair+key+smcintrapair+value+smcinterpair);
		if(i<data.size()){
			data.replace(data.begin()+i,data.begin()+i+key.size()+value.size()+2,nkey+smcintrapair+nvalue+smcinterpair);
			return true;
		}
		else return false;
	}

	//	inline bool hasKey(StringType &key) {return nextKey(key)<data.size();};
	//	inline bool hasValue(StringType &value) {return nextValue(value)<data.size();};
	//	StringType toString(){return data;}

	inline bool escape(StringType &key){
		bool b=smcreplaceAllInplace(key,smcintrapairunescaped,smcintrapairescaped,0,key.size());
		b=b&&smcreplaceAllInplace(key,smcinterpairunescaped,smcinterpairescaped,0,key.size());
		return b;
	};
	inline bool unescape(StringType &key){
		bool b=smcreplaceAllInplace(key,smcintrapairescaped,smcintrapairunescaped,0,key.size());
		b=b&&smcreplaceAllInplace(key,smcinterpairescaped,smcinterpairunescaped,0,key.size());
		return b;
	};
	inline unsigned int nextKey(StringType &key,unsigned int index=0){ // return fir
		//if(index>0) index--;
		unsigned int i=data.find(StringType()+smcinterpair+key+smcintrapair,index);
		if(i<data.size()) return i+1; else return i;
	};
	inline unsigned int nextKey(unsigned int index){
		if(index==0) return 1;
		else {
			unsigned int i=data.find(smcinterpair,index);
			if(i<data.size()) return i+1;
			else return i;
		}
	};
	inline unsigned int prevKey(unsigned int index){
		unsigned int i=data.rfind(smcinterpair,index);
		if(i<data.size()) return i+1;
		else return i;
	}
	inline unsigned int nextValue(StringType &value,unsigned int index=0){
		unsigned int i= data.find(StringType()+smcintrapair+value+smcinterpair,index+1);
		if(i<data.size()) return i+1; else return i;
	};
	inline unsigned int nextValue(unsigned int index=0){
		unsigned int i=data.find(smcintrapair,index);
		if(i<data.size()) return i+1;
		else return i;
	};
	inline unsigned int prevValue(unsigned int index=0){
		unsigned int i=data.find(smcintrapair,index);
		if(i<data.size()) return i+1;
		else return i;
	};
	StringType getNextValueForKey(StringType key, unsigned int &currentvalueindex){
		if(keys==0) return "";
		unsigned int i=nextKey(key,currentvalueindex);
		if(i<data.size()){
			i+=key.size()+1;
			currentvalueindex=i;
			return value(i);
		} else return "";	// should we update the indexes with -1 ?
	};
	unsigned int nextValueForKey(StringType &key,  unsigned int &currentvalueindex){
		if(keys==0) return -1;
		unsigned int i=nextKey(key,currentvalueindex);
		if(i<data.size()){
			i+=key.size()+1;
			currentvalueindex=i;
		}					// should we update the indexes with -1 ?
		return i;
	};

	StringType getNextKeyForValue(StringType &value,unsigned int &currentkeyindex){
		unsigned int i=nextKey(currentkeyindex);
		i=nextValue(value,i);
		if(i<data.size()){
			currentkeyindex=i=prevKey(i);
			return key(i);
		} else return "";
	};
	unsigned int nextKeyForValue(StringType &value,unsigned int &currentkeyindex){
		unsigned int i=nextKey(currentkeyindex);
		i=nextValue(value,i);
		if(i<data.size()) {i=prevKey(i);currentkeyindex=i;}
		return i;
	};

	bool isValue(unsigned int index){ // find if i is inside a value or a key
		unsigned int j=data.find(smcinterpair,index);
		if(j>=data.size()) return false;	// this is not supposed to happen ; last char is always interpair
		unsigned int i=data.find(smcintrapair,index);
		return j<i;
	}

	virtual inline StringType getPrevalueString(){return StringType()+smcintrapair;}; // get whatever mark a value start
	virtual inline StringType getPostvalueString(){return StringType()+smcinterpair;}; // get whatever mark a value end

	StringType getNextKeyPartialMatchingValue(StringType &value,unsigned int &index){
		unsigned int i=index+1,ds=data.size();
		while((i=data.find(value,i))<ds) {
			if(data[i]==smcintrapair) i++;
			if(isValue(i)) {
				unsigned int j=prevKey(i);
				StringType str=key(j);
				index=i;
				return str;// check if value
			}
			i++;

		}
		index=i;
		return "";
	};
	unsigned int nextKeyPartialMatchingValue(StringType &value,unsigned int &index){
		unsigned int i=index,ds=data.size();
		while((i=data.find(value,i))<ds) {
			if(data[i]==smcintrapair) i++;	// allow to search including the prevaluechar
			if(isValue(i)) {
				index=i;
				unsigned int j=prevKey(i);
				return j;// check if value
			}
			i++;
		}
		return i;
	};
	StringType getNextValuePartialMatchingKey(StringType &key,unsigned int &index, bool onrightkey=false){
		unsigned int i=index,ds=data.size();
		while((i=data.find(key,i))<ds) {
			if(!isValue(i)) {
				unsigned int j=nextValue(i);
				StringType str=value(j);
				index=j+str.size();
				return str;
			}
			i++;
		}
		return "";
	};

	unsigned int nextValuePartialMatchingKey(StringType &key,unsigned int &index, bool onrightkey=false){
		unsigned int i=index,ds=data.size();
		while((i=data.find(key,i))<ds) {
			if(!isValue(i)) {
				unsigned int j=nextValue(i);
				return j;
			}
			i++;
		}
		return i;
	};
};



#endif
