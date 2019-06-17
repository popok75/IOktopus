
	// Following 3 functions perform escaping/unescaping of json strings

std::string unescapeStringJSON(std::string jsonstring);


std::string escapeStringJSON(std::string jsonstring);

std::string makeEntry(std::string key, std::string value);






// a self allocated expandable templated list would be useful

//////////////////////////////////////////////////////////////////////////////////////////

#define ALLOCATEDSTEP 4

class JSONLoader {

public:

	std::string *json=0;
	unsigned int keyStart=0, keyEnd=0;
	unsigned int valueStart=0, valueEnd=0;
	unsigned int nextStart=0;

	bool cleanjson=false;
	std::string *err=0;

	char *closing=0;
	unsigned int closinglength=0,closingallocated=0;


	JSONLoader(std::string *json0, bool cleanjson0=false, std::string *err0=0):json(json0),cleanjson(cleanjson0),err(err0){};
	virtual ~JSONLoader(){if(closing) delete closing;};

	JSONLoader(const JSONLoader& orig) {	// deep copy
		json=orig.json;
		keyStart=orig.keyStart, keyEnd=orig.keyEnd;
		valueStart=orig.valueStart, valueEnd=orig.valueEnd;
		nextStart=orig.nextStart;
		cleanjson=orig.cleanjson;err=orig.err;

		closing=0;closinglength=0;closingallocated=0;
		for(unsigned int i=0;i<orig.closingallocated;i++) pushClosing(orig.closing[i]);
	};

	virtual std::string stringKey(){std::string k= json->substr(keyStart,keyEnd-keyStart);return unescapeStringJSON(k);}
	virtual std::string stringValue(){if(!(valueEnd>=valueStart)) return "";std::string v= json->substr(valueStart,valueEnd-valueStart);return unescapeStringJSON(v);}


	virtual bool openSubItemCallBack(){return true;};
	virtual bool closeSubItemCallBack(){return true;};
	virtual bool nextItem(){return true;}

	//	virtual void value( std::string key, std::string value){};
	virtual bool valueCallBack(){return true;};	// called when a new value is parsed
	virtual bool keyCallBack(){return true;};	// called when a new key is parsed


	void setJson(std::string *str){ json=str;};

	bool updateValue(unsigned int start, unsigned int end, unsigned int next ){
		valueStart=start;
		valueEnd=end;
		nextStart=next;
  		return valueCallBack(); // call loader callback
//		if(!cont) return false;
//		return true;
	};

	virtual void reset(std::string *json1=0){
		if(json1) json=json1;
		keyStart=0;
		keyEnd=0;
		valueStart=0;
		valueEnd=0;
		nextStart=0;
	}

	void pushClosing(char c){
		if(!closing || closinglength==closingallocated){	// expand allocated zone
			char *newptr=new char[closingallocated+ALLOCATEDSTEP];
			for(unsigned int i=0;i<closingallocated;i++) newptr[i]=closing[i];
			closingallocated+=ALLOCATEDSTEP;
			delete closing; closing=newptr;
		}
		closing[closinglength]=c;
		closinglength++;
	}
	char popClosing(){	// could contract it as well
		if(closinglength==0) return 0;
		closinglength--;
		return closing[closinglength];
	}
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#define INITALLOCATED 0
#define STEPALLOCATED 4

class HeavyJSONLoader : public JSONLoader {
	// heavy loader part
	unsigned int *higherIndexes=0;	// this + 2 next lines can be replaced by std::vector : tend to 8bytes + 4xn, usually bigger, n=10-20 > 24+4xn
	unsigned int depthAllocated=0; // to a table of current and previous value start
public:
	unsigned int depth=0;
	unsigned int order=0;
	bool keycalled=false;


	HeavyJSONLoader (std::string *json,bool cleanjson0=false, std::string *err0=0) : JSONLoader(json,cleanjson0,err0) {
		if (INITALLOCATED>0) reallocateAndCopy(INITALLOCATED);
	}
	virtual ~HeavyJSONLoader(){
		if(higherIndexes) delete higherIndexes;
	}

	void resetDepth(std::string *json1=0){
		if(higherIndexes) {delete higherIndexes;higherIndexes=0;}
		if (INITALLOCATED>0) reallocateAndCopy(INITALLOCATED);
		depth=0;depthAllocated=0;
	}

	HeavyJSONLoader(const HeavyJSONLoader& orig): JSONLoader(orig) {//deep copy
		order=orig.order;
		keycalled=orig.keycalled;
		higherIndexes=0;depth=0;depthAllocated=0;
		for(unsigned int i=0;i<orig.depth;i++) {
			push(orig.higherIndexes[(i*4)],orig.higherIndexes[(i*4)+1],orig.higherIndexes[(i*4)+2],orig.higherIndexes[(i*4)+3]);
		}
	};

	std::string stringKey(){
		std::string k= json->substr(keyStart,keyEnd-keyStart);
		if(k.empty()) return to_string(order);
		return unescapeStringJSON(k);}
	std::string stringValue(){if(!(valueEnd>=valueStart)) return "";std::string v= json->substr(valueStart,valueEnd-valueStart);return unescapeStringJSON(v);}


	virtual bool openSubItemCallBack(){
		push(keyStart,keyEnd,valueStart,order);
		order=0;keycalled=false;
		return true;};
	//	virtual void closeSubItem(std::string item){};

	virtual bool closeSubItemCallBack(){
		pop(keyStart, keyEnd, valueStart,order); //this restore the previous context
		rawValue();
		order++;
		return true;};

	virtual bool nextItem(){
		order++;
		return JSONLoader::nextItem();
	}


	// heavy loader part
	virtual bool rawValue(){return true;};	//rawvalue require a pile as a post call



	bool empty(){return depth==0;};
	void push(){push(keyStart,keyEnd,valueStart,order);};
	void pop(){pop(keyStart, keyEnd, valueStart,order);}

	void push(unsigned int &keystart, unsigned int &keyend, unsigned int &valuestart, unsigned int &ord){
		if(depth+1>depthAllocated) reallocateAndCopy(depth+1);
		higherIndexes[depth*4]=keystart;
		higherIndexes[depth*4+1]=keyend;
		higherIndexes[depth*4+2]=valuestart;
		higherIndexes[depth*4+3]=ord;

		//	std::cout << "HeavyJSONLoader push at depth " <<  depth << ": " <<keystart << ", " <<  keyend<< ", " <<  valuestart<< ", " <<  ord <<std::endl;

		depth++;
	};

	void valueAt(unsigned int dep, unsigned int &keystart, unsigned int &keyend, unsigned int &valuestart, unsigned int &ord){
		keystart=higherIndexes[dep*4];
		keyend=higherIndexes[dep*4+1];
		valuestart=higherIndexes[dep*4+2];
		ord=higherIndexes[dep*4+3];

		//	std::cout << "HeavyJSONLoader pop at depth: "<<  dep << ": "   <<  keystart << ", " <<  keyend<< ", " <<  valuestart<< ", " <<  ord <<std::endl;


	};
	void top(unsigned int &keystart, unsigned int &keyend, unsigned int &valuestart, unsigned int &ord){
		if(depth<1) return;
		valueAt(depth-1,keystart,keyend,valuestart,ord);
	};	// get top without popping
	void pop(unsigned int &keystart, unsigned int &keyend, unsigned int &valuestart, unsigned int &ord){
		if(depth<1) return;
		valueAt(depth-1,keystart,keyend,valuestart,ord);
		depth--;
	};

	inline bool hasArrayParent(){return (keyStart==keyEnd && valueStart>0);}

	std::string getPath(){
		std::string path="/";
		for(unsigned int i=depth;i>0;i--) {
			std::string key=json->substr(higherIndexes[4*(i-1)],higherIndexes[4*(i-1)+1]-higherIndexes[4*(i-1)]);
			if(key.empty()) key=to_string( higherIndexes[4*(i-1)+3]);
			if(key[0]=='"' && key[key.size()-1]=='"') key=key.substr(1,key.size()-2);	// if string remove ""
			path=std::string()+"/"+key+path;
		}
		return path;
	}


private:
	void reallocateAndCopy(unsigned int level){
		if(level>INITALLOCATED) level=level-level% STEPALLOCATED+STEPALLOCATED;
		else level=INITALLOCATED;
		if(level<=depthAllocated) return;
		unsigned int *newkeys=new unsigned int [level*4];
		for(unsigned int i=0;i<depth*4;i++) newkeys[i]=higherIndexes[i];
		if(higherIndexes) delete higherIndexes;
		higherIndexes=newkeys;
		depthAllocated=level;
	};
};
