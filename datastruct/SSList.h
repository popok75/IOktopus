
#ifndef StringType
#include <string>
#define StringType std::string
#endif


#define smcinterpair '\n'


////////////////////////////////////////////////////////////////////////////////////////////////////////
// StringList : vector of strings in a string
////////////////////////////////////////////////////////////////////////////////////////////////////////

class SSList {//vector of strings in a string
	StringType data=StringType()+smcinterpair;
	unsigned int values=0;
public:

	StringType get(unsigned int itemindex){
		unsigned int i=nextValue(0),ds=data.size();
		while(itemindex>0 && i<ds){i=nextValue(i+1);itemindex--;}
		if(i<ds) return value(i);
		else return "";

	};
	bool set(unsigned int itemindex, StringType value){//insert item at position 10, previous item is erased
		if(itemindex==values) push(value);
		unsigned int i=nextValue(0),ds=data.size();
		while(itemindex>0 && i<ds){i=nextValue(i+1);itemindex--;}
		if(i<ds) {
			unsigned int j=data.find(smcinterpair,i);
			data.erase(i,j-i);
			data.insert(i,value);
			return true;
		}
		else return false;
	};

	void insert(unsigned int itemindex, StringType value){	//insert item at position 10, previous item is pushed
		if(itemindex==values) push(value);
		unsigned int i=nextValue(0),ds=data.size();
		while(itemindex>0 && i<ds){i=nextValue(i+1);itemindex--;}
		if(i<ds) {
			data.insert(i,value+smcinterpair);
		}
	};


	unsigned int push(StringType value){
		data+=value+smcinterpair;
		values++;
		return values;
	};
	StringType pop(){
		unsigned int j=data.rfind(smcinterpair,data.size()-1);
		StringType val=data.substr(j,data.size()-1);
		data.erase(j,data.size()-1);
		values--;
		return val;
	};

	unsigned int nextValue(StringType &value,unsigned int charpos){
		unsigned int i= data.find(StringType()+smcinterpair+value+smcinterpair,charpos);
		if(i<data.size()) return i+1; else return i;
	}
	unsigned int nextValue(unsigned int charpos){
		unsigned int i= data.find(smcinterpair,charpos);
		if(i<data.size()) return i+1; else return i;
	};

	inline StringType value(unsigned int charpos){
		if(data[charpos-1]!=smcinterpair) return "";
		unsigned int j=data.find(smcinterpair,charpos);
		return data.substr(charpos,j-charpos);
	}

	inline unsigned int size(){return values;}
	inline unsigned int getByteSize(){return data.size();}
	inline StringType *getData(){return &data;}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef INTOSTRINGFUNCS
#define INTOSTRINGFUNCS
template <typename T> StringType intoString(T ptr){return StringType((char *)&ptr,sizeof(ptr));}	// store any type in a string


template <typename T> T intoType(std::string strval){	//restore the type (limited to 64bit max)	// I see no escaping, no check, are we crazy ?
	uint64_t result=0,f=1;	//	unsigned int s=strval.size();
	for(unsigned char c :strval){
		result+=f*c;
		f*=0x100;
	}
	return (T)result;
};
#endif
