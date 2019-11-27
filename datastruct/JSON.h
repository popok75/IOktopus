#ifndef JSON_H
#define JSON_H
#define DEBUG_VERBOOSE 0


#include "JSONLoaders.h"


#include <string>
//#include <iostream>	//workaround: A _putc_r definition is found on line 96 of packages/esp8266/hardware/esp8266/2.4.1/cores/esp8266/libc_replacements.c Simply commenting it out allowed me to compile.

/*
 	 // This file is highly chaotic right now.
 	 // It mixes c style function call based parser with c++ style iterator object for callback JSONLoader & HeavyJSONLoader
 	 // the following would help
 	 //		- a better separation between main functions, other json specific, helper functions & loader
 	 //		- better encapsulation of loader (no public variables)
 	 //		- add more comments, not very clear how it is expected to behave and be used
 	 //		- improve adherence to keywords from https://www.json.org/
*/

// example of use
/*
class MyHeavyJSONLoader: public HeavyJSONLoader {
public:
	GenTreeMapHeavyJSONLoader (std::string *json, bool cleanjson0=false, std::string *err0=0):HeavyJSONLoader(json,cleanjson0,err0), map(map0){};
	virtual bool valueCallBack(){
		GenString path=getPath()+stringKey();
		std::cout << "MyHeavyJSONLoader:"<<path << ":" << stringValue() << std::endl;
		return JSONLoader::valueCallBack();
	};
};


class MyJSONLoader: public JSONLoader {
public:
	MyJSONLoader (std::string *json, bool cleanjson0=false, std::string *err0=0):JSONLoader(json,cleanjson0,err0){};
	virtual bool valueCallBack(){
		std::cout << "MyJSONLoader key :'"<<stringKey() << "', val:'"<<stringValue()<<"'"<< std::endl;
		return JSONLoader::valueCallBack();
	};
	virtual bool keyCallBack(){
		std::cout << "MyJSONLoader key :'"<<stringKey() << "', val:'"<<stringValue()<<"'"<< std::endl;
		return JSONLoader::keyCallBack();
	};
};


void main(){
	std::string jsoncontent="{\"XXX\":{\"hello\":\"123\"},\"bonjour\":\"321\"}";
	MyHeavyJSONLoader heavyloader(&jsoncontent);
 	MyJSONLoader loader(&jsoncontent);
	parseJSON(&loader);
	std::cout << "testIODatav03: executed 1" << std::endl;
	parseJSON(&heavyloader);
	std::cout << "testIODatav03: executed 2" << std::endl;
}

*/


class JSONLoader;

// parseItemJson is the central function that allows parsing an item from start to end
bool parseItemJSON(std::string &json, unsigned int &indexp, JSONLoader *loader,unsigned int *valueend=0);

// parseEntryJSON is the second most important that allows parsing an entry start
bool parseEntryJSON(std::string &json, unsigned int &indexp, JSONLoader *loader,unsigned int *valueend =0);

// top level functions using parseItem
bool cleanJSON(std::string &json);
bool checkSyntaxJSON(std::string &json,std::string *err=0);
bool parseJSON( std::string &jsoncontent, JSONLoader *jsonloader, std::string *err=0); // provide json content, a loader, optionally an error string

unsigned int nextNonCommentFieldStart(std::string &json,unsigned int start);
bool cleanJSONPart(std::string &json,unsigned int index1,unsigned int index2);
inline unsigned int findFieldClosing(std::string &json, unsigned int indexp);





bool cleanJSON(std::string &json){ //remove all comments and spaces

	// remove all comments unless inside ""
	unsigned int indexp=0;

	// comments inside fields are not removed
	// fields inside comments are removed

	unsigned int index=nextNonCommentFieldStart(json,indexp);

	while(index<json.size()) {
		bool b=cleanJSONPart(json,indexp,index);
		if(b) index=json.find("\"",indexp);//json.find("\"",indexp);	// if we changed the string, remake the field start search
		index++;
		index=findFieldClosing(json,index);
		if(index>=json.size()) return false; //error here

		indexp=index+1;
		index=nextNonCommentFieldStart(json,indexp);// json.find("\"",indexp); // find next field start
	}
//	bool b=
			cleanJSONPart(json,indexp,json.size());
#if DEBUG_VERBOOSE>0
	std::cout <<"cleanJSON cleaned with success:"+json << std::endl;
#endif

	return true;

};


/*
static inline void ltrimc(std::string &s, char c=' ') {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [c](int ch) {if(c==' ') return !std::isspace(ch); else return !(c==ch);
	}));}
// trim from end (in place)
static inline void rtrimc(std::string &s, char c=' ') {
	s.erase(std::find_if(s.rbegin(), s.rend(), [c](int ch) { if(c==' ') return !std::isspace(ch); else return !(c==ch);
	}).base(), s.end());}
// trim from both ends (in place)
static inline void trimc(std::string &s, char c=' ') {ltrimc(s,c);rtrimc(s,c);}	//remove chars at start or end that match the char
*/

//const char *escapesv2="\b\r\t\n\\\"";

char escapes[]={'\b','\r','\t','\n','\"','\\'};// PROBLEM here with memory ?, too many chars?, all should be RF or progmem ?
char unescapes[]={'b','r','t','n','"','\\'};
char escapechar ='\\',otherescape='/';
unsigned int escapecharnumber=6;

inline std::string unescapeStringJSON(std::string jsonstring){	// \u+4hexa not supported
	unsigned int s=jsonstring.size()-1;
	if(jsonstring[0]!='"' || jsonstring[s]!='"') return jsonstring;//check is it is a json string
	//jsonstring=jsonstring.substr(1,jsonstring.size()-2);	// remove first " and last "
	jsonstring.erase(s);
	jsonstring.erase(0,1);


	unsigned int index=0;
	while((index=jsonstring.find(escapechar,index))<jsonstring.size()-1){
		if (jsonstring[index+1]==otherescape) {jsonstring.erase(index);}
		else {
			for(unsigned int j=0;j<escapecharnumber;j++)
				if(jsonstring[index+1]==unescapes[j]) {jsonstring.erase(index,1);jsonstring[index]=escapes[j];break;}
		}
		index++;
	}

	return jsonstring;
}


inline std::string escapeStringJSON(std::string jsonstring){	// \u+4hexa not supported

	unsigned int index=0;
	while((index=jsonstring.find_first_of(escapes,index))<jsonstring.size()){
		for(unsigned int j=0;j<escapecharnumber;j++)
			if(jsonstring[index]==escapes[j]) {
				jsonstring.insert(index,1,escapechar);
				jsonstring[index+1]=unescapes[j];
				index++;
				break;}
		index++;
	}
	jsonstring.insert(0,1,'\"');
	jsonstring.insert(jsonstring.size(),1,'\"');
	return jsonstring;
}

inline std::string makeEntry(std::string key, std::string value){
	return key+":"+value;
}





//////////////////////////////////////////////////////////////////////////////////////////
// count \n from 0 to max index
unsigned int countLines(std::string &s, unsigned int max ) {
	unsigned int count = 1, index=0;
	while(index<max){index=s.find("\n",index+1);if(index<max) count++;}
	return count;
}

// replace once in same string all occurrence of pattern by newval from start to end index
inline bool jsonreplaceAllInplace(std::string &src, std::string &pattern,std::string &newval,std::string::size_type start, std::string::size_type end){
	bool changed=false;
	while ( ( start = src.find( pattern, start ) ) < end )
	{
		src.replace( start, pattern.size(), newval );
		changed=true;
		start += newval.size();
	}
	return changed;
}
std::string jsonto_string(uint64_t num){
	uint8_t i = 0;  uint64_t n = num;
	char str[21];
	do i++;
	while ( n /= 10 );

	str[i] = '\0';
	n = num;

	do str[--i] = ( n % 10 ) + '0';
	while ( n /= 10 );

	return std::string(str);
};
/*
// replace in a new string all occurrence of pattern
std::string jsonreplaceAll(std::string src, std::string pattern,std::string newval){
	replaceAllInplace(src,pattern,newval,0,src.size());
	return src;
}
 */

std::string RN="\r\n",LN="\n";

// print an extract of the json at specific index
std::string printExtract(std::string &json, unsigned int index0){
	unsigned int usize=40,index=index0,index2=index0;
	if(index>usize) index-=usize;
	else index=0;
	if(json.size()>(index0+usize)) index2=index0+usize;
	else index2=json.size();
	std::string ret="\n>>";
	if(index0>index) ret+=json.substr(index,index0-index);
	ret+=">><<";
	if(index2>index) ret+=json.substr(index0,index2-index0);
	ret+="<<";
	jsonreplaceAllInplace(ret,RN,LN,0,ret.size());
	return ret;
}

// print error message
inline void printErrorJSON(std::string symbol, std::string &json, unsigned int indexp, std::string *err, std::string mess=""){
	if(!err) return ;
	if(mess=="") mess="Syntax Error: can't find ";
	*err+=mess+"'"+symbol+"' expected at line :"+ to_string(countLines(json,indexp))+" :"+printExtract(json,indexp);
};

// find next non whitespace character
char spacechars[]={' ','\n','\t','\r'};
inline unsigned int nextNonSpace(std::string &str, unsigned int index0){
	for(;index0<str.size();index0++){
		bool isspace=false;
		for(char c:spacechars) if(str[index0]==c){isspace=true;break;}
		if(!isspace) return index0;
	}
	return index0;
}

// find next non whitespace and non comment character
inline unsigned int nextNonSpaceNonComment(std::string &str, unsigned int index0){
	unsigned int index=nextNonSpace(str,index0);
	while(index<str.size()){
		if(str[index]=='/' && index+1<str.size()){
			if(str[index+1]=='*') index=str.find("*/",index+2)+2;
			if(str[index+1]=='/') index=str.find("\n",index+2)+1;
		} else return index;
		index=nextNonSpace(str,index);
	}
	return index;
}





//////////////////////////////////////////////////////////////////////////////////////////

inline bool isSpace(std::string &json, unsigned int indexp){for(char c:spacechars) if(json[indexp]==c){return true;}return false;};
inline bool isString(std::string &json, unsigned int indexp){return json[indexp]=='"';};
inline bool isNumber(std::string &json, unsigned int indexp){char c=json[indexp];return (c>='0' && c<='9') || c=='-' || c=='.';};
inline bool isObject(std::string &json, unsigned int indexp){return json[indexp]=='{';};
inline bool isArray(std::string &json, unsigned int indexp){return json[indexp]=='[';};

inline bool isComma(std::string &json, unsigned int indexp){return json[indexp]==',';};
inline bool isClosing(std::string &json, unsigned int &indexp){return json[indexp]=='}'||json[indexp]==']';};

inline bool compareChars(const char *src,const char*dst,unsigned int length){for(unsigned int i=0;i<length;i++) {if(src[i]!=dst[i]) return false;}return true;};
inline bool isTrue(std::string &json, unsigned int &indexp){
	//unsigned int s=json.size();
	if((indexp+4)>=json.size()) return false;
	return compareChars("true",json.c_str()+indexp,4);}
inline bool isFalse(std::string &json, unsigned int &indexp){if(indexp+4>=json.size()) return false;return compareChars("false",json.c_str()+indexp,5);}
inline bool isNull(std::string &json, unsigned int &indexp){if(indexp+4>=json.size()) return false;return compareChars("null",json.c_str()+indexp,4);};



// compare if indexp is incremented to first nonspace and return if content is a symbol (or symbol2 if defined),
inline bool checkIsAt(char symbol,std::string &json, unsigned int &indexp, std::string *err, char symbol2=0, char symbol3=0) {
	unsigned int index=nextNonSpaceNonComment(json,indexp);
	if(index<json.size())
		if(index>=json.size() || (json[index]!=symbol && (symbol2==0 || json[index]!=symbol2) && (symbol3==0 || json[index]!=symbol3)) ) { //different from symbol1 and from symbol2 if not 0
			if(err) {
		//		char c=json[index];	// toremove
		//		std::string str=json.substr(index,20);
				std::string mess=std::string() +symbol;
				if(symbol2) mess+=std::string()+"' or '"+symbol2;
				if(symbol3) mess+=std::string()+"' or '"+symbol3;
				printErrorJSON(mess,json,indexp,err);
			}
			return false;
		}
	indexp=index;
	return true;
};

inline bool isEscaped(std::string &json, unsigned int indexp){	// e.g. \" -> true , \\" -> false
	unsigned int i=indexp;
	for(;i>0;i--) if(json[i-1]!='\\') break;
	i=indexp-i;
	if((i%2)==0) return false;
	else return true;
}

inline unsigned int findFieldClosing(std::string &json, unsigned int indexp){

	// 2 methods: look for " and count the \ before, or look for \ and add 2
	unsigned int index=json.find('\"',indexp);
	while (index<json.size() && isEscaped(json,index))
		index=json.find('\"',index+1);
	return index;
}


// parse a json "string" field starting at indexp
inline bool parseFieldJSON(std::string &json, unsigned int &indexp, JSONLoader *loader=0){
	if(!isString(json,indexp)) return false;
	indexp++;
//	std::cout << json.substr(indexp,20)<<std::endl;
	unsigned int index=findFieldClosing(json,indexp);
	// escaped chars : here we must check that it is a ", or pair \\", and not impair \", or \\\", which is allowed
	// should check if each escaped is correct as well in debug mode
	//	if(!loader->cleanjson && !checkIsAt('"',0,json,indexp,loader->err)) return false;
	if(index>=json.size()) {printErrorJSON("\\",json,indexp,loader->err);return false;}

	indexp=index+1;
	return true;
}


// parse a json "string" field starting at indexp
inline bool parseNumberJSON(std::string &json, unsigned int &indexp, JSONLoader *loader=0){
	// TODO : extend to include scientific notation, e.g. 10e3
	if(!isNumber(json,indexp)) return false;
	unsigned int index=indexp;

	if(!loader || loader->cleanjson) while(isNumber(json,index)) index++;
	else {
		bool dot=false;
		while(isNumber(json,index)) {
			if(json[index]=='-' && index>indexp){printErrorJSON("-",json,indexp,loader->err,"Symbol misplaced");return false;}
			if(json[index]=='.'){
				if(dot){printErrorJSON(".",json,indexp,loader->err,"Symbol redundant");return false;}
				dot=true;
			}
			index++;
		}
	}
//	char c=json[index];		//  TODO: to remove
//	std::string str=json.substr(index,20);
	if(loader && !loader->cleanjson){
		index=nextNonSpaceNonComment(json,index);
		if((index>=json.size() ||
				(!isSpace(json,index) && !checkIsAt(':',json,index,loader->err,',',']')) ))
		{printErrorJSON(std::string()+json[index],json,index,loader->err,"\nSymbol misplaced");return false;}
	}

	indexp=index;
	return true;
}


// parse a json "string" field starting at indexp
inline bool parseBoolNullJSON(std::string &json, unsigned int &indexp, JSONLoader *loader=0){
	if(isFalse(json,indexp)){indexp+=5;return true;}
	if(isTrue(json,indexp) || isNull(json,indexp)){
		indexp+=4;
		return true;
	}
	return false;
}
/*
bool updateLoaderValue(std::string &json,unsigned int index, unsigned int &indexp, JSONLoader *loader){
	loader->valueStart=index;
	loader->valueEnd=indexp;
	loader->nextStart=indexp;
	//next start will point just after close symbol
	if(checkIsAt(',',json,loader->nextStart,0)) loader->nextStart++;	// unless there is a , in that case it will be after the ,
//	char c=(*loader->json)[loader->nextStart];	//  TODO: to remove
	bool cont=loader->valueCallBack(); // call loader callback
	if(!cont) return false;
	return true;
};
*/

bool nextItemStart(std::string &json,unsigned int indexp){
	//next start will point just after close symbol
	if(checkIsAt(',',json,indexp,0)) indexp++;	// if there is a , in that case it will be after the ,
	return indexp;
};

// parse a json value starting at indexp
bool parseValueJSON(std::string &json, unsigned int &indexp, JSONLoader *loader=0, unsigned int *valueend =0){//char c=json[indexp];
	if(loader && !loader->cleanjson) indexp=nextNonSpaceNonComment(json,indexp);
	unsigned int index=indexp;
	//	char c=json[indexp];	//  TODO: to remove
	bool b=parseFieldJSON(json,indexp);
#if DEBUG_VERBOOSE > 0
	std::cout << "parseValueJSON parsed value :'" <<json.substr(index,indexp-index) <<"'"<< std::endl;
#endif
	/*str=json.substr(index,indexp-index);
	std::cout << "parseValueJSON parsed value2 :'" <<str <<"'"<< std::endl;*/
	//	c=json[indexp];		//  TODO: to remove
	if(!b && (isObject(json,indexp) || isArray(json,indexp)) )  { // in case value is a an object
		if(loader)	{loader->valueStart=indexp;};	//set loader to start at that position
		return true;
	}
	if(!b) b= parseNumberJSON(json,indexp,loader);
	if(!b) b= parseBoolNullJSON(json,indexp,loader);
	if(!b) return false;	// if type not found or parsed wrong
#if DEBUG_VERBOOSE>1
	std::cout << "parseValueJSON value :'"<<json.substr(index,indexp-index) <<"'"<< std::endl;

#endif

	if(loader && !loader->updateValue(index,indexp,nextItemStart(json,indexp))) return false;// here we update the value field : from index+1 to indexp

	if(loader && !loader->cleanjson) indexp=nextNonSpaceNonComment(json,indexp);// move cursor to next non space

	if(valueend) *valueend=indexp;

	return true;

}





// parse object or array entry once without subobject or subarray
// entry can be a string:value pair or just a value
bool parseEntryJSON(std::string &json, unsigned int &indexp, JSONLoader *loader,unsigned int *valueend ) {
	if(loader && !loader->cleanjson) indexp=nextNonSpaceNonComment(json,indexp);
	//	char c=json[indexp];
//	StringType	str=json.substr(indexp,20);
	if(!isString(json,indexp))	//are we on a field ? if not we are on a value and there is no key, right ?
	{
		bool b=parseValueJSON(json,indexp, loader,valueend);
		if(loader) {loader->keyStart=loader->valueStart;loader->keyEnd=loader->valueStart;}// no key
		//		c=json[indexp];		//  TODO: to remove
		// std::string str=json.substr(indexp,20);		//  TODO: to remove
		if(checkIsAt(',',json,indexp,0)){
			indexp++;
			if(loader && !loader->cleanjson) {
				indexp=nextNonSpaceNonComment(json,indexp);
				if(isClosing(json,indexp)) {printErrorJSON("} or ]",json,indexp,loader->err,"Syntax Error:  after ',' closing symbol");return false;} // check that following the , there is no closing
			}
		}
		return b;
	} else { // if string can be a key or a value
		unsigned index=indexp;
		//	c=json[indexp];	//  TODO: to remove
		bool b= parseFieldJSON(json,indexp,loader);
		if(!b) return false;

		// if array here can be a closing ] as well but not object closing }
		//		c=json[indexp];		//  TODO: to remove
		//		str=json.substr(indexp,20);		//  TODO: to remove

		unsigned int keyend=indexp;
		if(loader && !loader->cleanjson && !checkIsAt(':',json,indexp,loader->err,',',']')) return false;
		bool objectparent=(json[indexp]==':');

#if DEBUG_VERBOOSE>1
		if(objectparent) std::cout << "parseEntryJSON object key :'"<<json.substr(index,indexp-index) <<"'"<< std::endl;
		else std::cout << "parseEntryJSON array value :'"<<json.substr(index,indexp-index) <<"'"<< std::endl;

#endif
		if(loader){
			loader->reset(&json);	//should we reset in case of value and not key?
			if(objectparent){	//means parent is object
				loader->keyStart=index;loader->keyEnd=keyend;// here we can get the key field value from index+1 to indexp
				bool cont=loader->keyCallBack(); // call loader callback
				if(!cont) return false;
			} else{
				if(keyend!=indexp){	// maybe here too we need to use
				//	std::cout << "problem here ?"<< std::endl;
				}
				bool b=loader->updateValue(index,keyend,nextItemStart(json,indexp));//updateLoaderValue(json,index,indexp,loader);
				if(!b) return false;
			}
		}





		if(objectparent){
			indexp++;
		//	loader->keyStart=indexp;loader->keyEnd=indexp;
			//			char c2=json[indexp];
			//			std::string str2=json.substr(indexp,20);
			b= parseValueJSON(json,indexp, loader,valueend);
			if(!b) return false;
		}
		if(loader && !loader->cleanjson && *valueend>0 && !checkIsAt(',',json,indexp,loader->err,']','}')) return false;	// if we parsed a value, then we must be at a , or a closing
		//		c=json[indexp];		//  TODO: to remove
		//		str=json.substr(indexp,20);		//  TODO: to remove
		if(json[indexp]==',') {
			indexp++;
			if(loader) loader->nextItem();
			if(loader && !loader->cleanjson && isClosing(json,indexp)) {printErrorJSON("} or ]",json,indexp,loader->err,"Syntax Error:  after ',' closing symbol");return false;} // check that following the , there is no closing
		}
		return true;
	}


}

char closeLoader(std::string &json, unsigned int &indexp, JSONLoader *loader){
	char c=loader->popClosing();
	if((json[indexp]=='}' && c=='{') ||
			(json[indexp]==']' && c=='[')) return 0;//found
	return c;//not found
}






bool openItemJSON(std::string &json, unsigned int &indexp, JSONLoader *loader){
	bool b=true;
	if(loader) {
		if(!loader->cleanjson) loader->pushClosing(json[indexp]);
#if DEBUG_VERBOOSE>1
		std::cout << "parseItemJSON open subitem  " << std::endl;
#endif
		b=loader->openSubItemCallBack();
	}
	indexp++;
	if(loader && isClosing(json,indexp)){	// this prevent the loader to keep previous data if we have void item
		loader->reset();
		loader->valueStart=loader->nextStart=indexp;
	}
	return b;
}

bool closeItemJSON(std::string &json, unsigned int &indexp, JSONLoader *loader,unsigned int *valueend =0){
	bool b=true;
	if(loader && !loader->cleanjson) {
		char c2=closeLoader(json,indexp,loader);
		if(c2!=0) {
			printErrorJSON(std::string()+c2,json,indexp,loader->err,"Syntax Error: closing symbol dont match opening symbol");
			return false;}// correct closing must be tested according to loader
	}
#if DEBUG_VERBOOSE>1
	std::cout << "parseItemJSON close subitem  " << std::endl;
#endif
	indexp++;
	if(loader) {loader->valueEnd=indexp;} //restore valueEnd	// this line push valueend beyond value end, why ?
																// what are we restoring ?
	if(valueend) *valueend=indexp;
	if(loader && !loader->cleanjson) {
		indexp=nextNonSpaceNonComment(json,indexp);
		if(!checkIsAt(',',json,indexp,loader->err,']','}')) return false;
	}
	if(json[indexp]==',') {	// skip the , and check that no closing is next (ok for the parser but not ok syntaxically)
		indexp++;
		if(loader && !loader->cleanjson && isClosing(json,indexp)) {printErrorJSON("} or ]",json,indexp,loader->err,"Syntax Error:  after ',' closing symbol");return false;} // check that following the , there is no closing
	}
	if(loader) {loader->nextStart=indexp;loader->closeSubItemCallBack();} // call loader once valueend and nextstart are set
	return b;
}

///////// Parse a json item until finished: object or array
// normal case: {"abc",12,false}
// special case: {}, []
// error case : {"item",},
// worst error case : ["item":value]
bool parseItemJSON(std::string &json, unsigned int &indexp, JSONLoader *loader,unsigned int *valueend) {
	// position at entry start
	if(loader && !loader->cleanjson){
		if(!checkIsAt('{',json,indexp,loader->err,'[')) return false;
		loader->pushClosing(json[indexp]);
	}
	indexp++;
	unsigned int  depth=0;
	unsigned int valueendinst=0;
	if(!valueend) valueend=&valueendinst;
	while(indexp<json.size()){ // the loop must end when reaching the closing } of current depth
		if(loader && !loader->cleanjson) indexp=nextNonSpaceNonComment(json,indexp);
#if DEBUG_VERBOOSE>2
		char c=json[indexp];//TODO: remove this line
		std::string str=json.substr(indexp,20);
#endif
		if(isClosing(json,indexp)) { //no item or after last item // if true after , error triggered in parseEntry
#if DEBUG_VERBOOSE>2
			str=json.substr(indexp,20);
#endif
			closeItemJSON(json,indexp,loader,valueend);
			if(depth==0) return true;
			depth--;
			continue;
		}
		*valueend=0;
		bool b=parseEntryJSON(json,indexp, loader,valueend);
		if(!b) return false;

		if(*valueend==0){ // value was not parsed
#if DEBUG_VERBOOSE>1
		//	char c=json[indexp];	//TODO: remove this
		//	std::string str=json.substr(indexp,20);

#endif
			// next test is redundant, if value not parsed, it is necessarily a subitem
			if(isObject(json,indexp) || isArray(json,indexp)) { // next char is new object or array
				openItemJSON(json,indexp,loader);
				depth++;
			}
		}
	}
	printErrorJSON("}",json,indexp,loader->err,"Syntax Error: can't find closing symbol");
	return false;
}



bool skipItemJSON(std::string &json, unsigned int &indexp, JSONLoader *loader){
	unsigned int valend;
	bool b=parseItemJSON(json,indexp,0,&valend);
	if(loader && b) {
		loader->valueEnd=valend;
		loader->nextStart=indexp;
		//std::cout << "parseItemJSON skip subitem  " << json.substr(loader->valueStart,loader->valueEnd-loader->valueStart) << std::endl;
	}
	return b;
}










bool cleanJSONPart(std::string &json,unsigned int index1,unsigned int index2){

	unsigned index=json.find("/*",index1),index2c=index2;

	while(index<index2c){
		unsigned int eindex=json.find("*/",index+2);
		if(eindex<index2c) {
			unsigned int cl=2+eindex-index;
			json.erase(index,cl);
			index2c-=cl;
		} // if not true there is an error
		index=json.find("/*",index1);
	}

	index=json.find("//",index1);	// find starting comment
//	StringType str;
//	if(index<json.size()) str=json.substr(index,100);
	while(index<index2){
		unsigned int eindex=json.find("\n",index+2);	// find end of comment
		if(eindex<index2) {
			unsigned int cl=1+eindex-index;
			index2-=cl;
			json.erase(index,cl);
		} // if not true there is an error
		index=json.find("//",index1);
	}

	std::string nothing="";
	for(char c:spacechars) {
		std::string cstr=nothing+c;
		jsonreplaceAllInplace(json,cstr,nothing,index1,index2);}
	return true;
};

unsigned int nextNonCommentFieldStart(std::string &json,unsigned int start){
	unsigned index=json.find("\"",start);
	unsigned indexc1=json.find("/*",start);
	unsigned indexc2=json.find("//",start);
	while(indexc1<index || indexc2<index) {
		if(indexc1<index) index=json.find("*/",indexc1)+2;
		if(indexc2<index) index=json.find("\n",indexc2)+1;
		indexc1=json.find("/*",index);
		indexc2=json.find("//",index);
		index=json.find("\"",index);
	}
	return index;
}

bool checkSyntaxJSON(std::string &json,std::string *err){
	std::string errstr;
	unsigned int index=0;
	if(!err) err=&errstr;

	JSONLoader *jsonloader=new JSONLoader(&json,false,err);

	bool b= parseItemJSON(json,index,jsonloader);
#if DEBUG_VERBOOSE>1
	if(!b) {
		std::cout <<"checkSyntaxJSON failed :\n"+*err << std::endl;
	}
	else std::cout <<"checkSyntaxJSON success !\n"+*err << std::endl;
#endif

	delete jsonloader;
	return b;
};


bool parseJSON(std::string &jsoncontent, JSONLoader *jsonloader, std::string *err){
	std::string errstr;
	unsigned int index=0;
	if(!err) err=&errstr;

	if(jsonloader) jsonloader->setJson(&jsoncontent);
	bool b= parseItemJSON(jsoncontent,index,jsonloader);
#if DEBUG_VERBOOSE>1
	if(!b) {
		std::cout <<"checkSyntaxJSON failed :\n"+*err << std::endl;
	} else std::cout <<"checkSyntaxJSON success !\n"+*err << std::endl;
#endif

	return b;
};




// "/action/newop/key" "strval" -> "action":{"newop":{key:"strval"}}
// "/action/0/key" "strval" -> "action":[{key:"strval"}]
std::string pathValToEntry(std::string path, std::string val, bool lastisobject0){
	//bool lastisobject=lastisobject0;
	unsigned int end, pstart=0;
	end=path.find("/",pstart+1);
	std::string mess,close;
	bool run=true,dot=false;
	while(run) {std::string key=path.substr(pstart+1,end-pstart-1);
	if(end<path.size()){
		bool num=true;
		for(unsigned int i=0;i<key.size();i++){
			/*		unsigned char c=key[i];
				bool b=!isNumber(key,i);
				bool b1=(i>0 || key[i]!='-');
				bool b2=(key[i]!='.' || dot);
			 */	if(!isNumber(key,i) && (i>0 || key[i]!='-') && (key[i]!='.' || dot)) {num=false;break;}
			 if(key[i]=='.') dot=true;
		}
		if(num){
			mess+="[";close="]"+close;
		//	lastisobject=false;
		} else{
			key="\""+key+"\"";
			if(lastisobject0) mess+=key+":";
			else {mess+="{"+key+":";close="}"+close;}
			lastisobject0=false;
		//	lastisobject=true;
		}
		pstart=end;
		end=path.find("/",pstart+1);
	}
	else {
		if(pstart+1<path.size()){// path end with a key
			if(!lastisobject0) {mess+="{" ;close="}"+close;}
			key="\""+key+"\"";
			mess+=key+":"+val;

		} else {
			mess+=val;
		}
		run=false;
		break;
	}
	}
	//	mess=mess.substr(1,mess.size()-1);
	return mess+close;
}


#endif
