#ifndef DATAENCODER_H
#define DATAENCODER_H


#include "../../datastruct/GenString.h"
#undef MILLISFACTOR
#define MILLISFACTOR 1000 	//use seconds

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.dataencoder"


////////////////////////////////////////////////
class DataEncoder{
public:
//	DataEncoder(){};
};

////////////////////////////////////////////////
class JsonEncoder : public DataEncoder{
	/*
	 	 	 The data encoding is the following:
	 	 	 	 - info insert :
	 	 	 	 	 1 var colpername: "{VAR0NAME:0,VAR1NAME:1,VAR2NAME:2}"
	 	 	 	 	 2 stream start : [newstream]
					  	  - if a new stream add '[newstream],', file can end without interrupting the stream
	 	 	 	 	 3 timestamp tag : {sync:absts},{unsync:absts},{resync:diffts}
	 	 	 	 	 	 - absts becomes reference for relts
	 	 	 	 	 	 - synced is after NTP sync, unsync is a best guess, resync applies retrospectiely until start of stream

				 4 normal data is "[relts,0,valuen0,1,valuen1]"
				 	 - insert leave the dataline open in case another data come in later for the same ts


				 5 end if stream is "]"

			 N.B: relative is the difference with last absolutetimestamp. it is allowed to have duplicate lines, such as' [0,1,18.2],[0,2,44.93]", including with info insert in between

			The best order is :
 	 	 	 	 1. colpername line
 	 	 	 	 2. stream start
 	 	 	 	 3. timestamp tag
 	 	 	 	 4. data lines
 	 	 	 	 i-1. timestamp tag (resync)
 	 	 	 	 i. data lines
 	 	 	 	 n. end

	 */
	bool started=false,newstream=true,lineopen=false;
	uint64_t lastTimestamp=0;
	uint64_t referenceTimestamp=0;
	std::vector<GenString> varnames;


public:
	JsonEncoder(){};

	bool getNewstream(){return newstream;}

	bool preloadVarname(GenString varname){
		bool found=false;
		for(unsigned int i=0;i<varnames.size();i++) if(varnames[i]==varname) {found=true;}
		if(!found) {varnames.push_back(varname);return true;}
		return false;
	}

	std::vector<unsigned char> getBytes(uint64_t absolutetimestamp,std::vector<GenString> varnames, std::vector<ValType> values){return std::vector<unsigned char>();};
	std::vector<unsigned char> getBytes(uint64_t absolutetimestamp,GenString varname, ValType value){
		GenString bytes ;
		GenString op=RF("["),cl=RF("]"),
				opobj=RF("{\""),clobj=RF("}"), inter=RF("\":"),
				opsync=RF("{\"sync\":");
		bool found=false;unsigned int index=-1;
		for(unsigned int i=0;i<varnames.size();i++) if(varnames[i]==varname) {found=true;index=i;}
		if(!found) {// first time :[newstream],{varname:0},{sync:absolutetimestamp},[0,1,value]
			index=varnames.size();
			varnames.push_back(varname);// first time copy varname to local vector
		}
		if(!started) {	// write a header
			bytes+='[';// if file exist we need to close and reopen
			if(newstream) {bytes+=RF("[\"newstream\"],");newstream=false;}
			bytes+=opobj;
			for(unsigned int i=0;i<varnames.size();i++) {if(i!=0) bytes+=RF(",\"");bytes+=varnames[i]+ inter+to_string(i);}
			bytes+=clobj+',';
			bytes+=opsync+to_string(absolutetimestamp/MILLISFACTOR)+clobj+',';
			referenceTimestamp=absolutetimestamp;
		} else if(!found){
			if(lineopen) bytes+=cl+',';
			bytes=opobj+varname+ inter+to_string(index)+clobj+','; //later on add only new varname
		}

		if(started && lastTimestamp/MILLISFACTOR!=absolutetimestamp/MILLISFACTOR) {bytes+=cl+',';lineopen=false;}
		if(!lineopen) {
			uint64_t relts=absolutetimestamp-referenceTimestamp;

			bytes+=op+to_string(relts/MILLISFACTOR);
		}
		bytes+=','+to_string(index)+','+to_stringWithPrecision(value,2);
		lineopen=true;
		lastTimestamp=absolutetimestamp;
		started=true;
		println(GenString()+"bytes:"+bytes);

		// then produce the header

		// produce the data
			// if same timestamp add to it
			// if not close and open new timestamp+values, do not close

		return std::vector<unsigned char>(bytes.begin(),bytes.end());
	};


	std::vector<unsigned char> getResyncBytes(uint64_t timestamp){return std::vector<unsigned char>();};	// is called when clock detect a resync

	std::vector<unsigned char> getEndBytes(){
		return std::vector<unsigned char>({']',']'});
	};	// is called w

	uint64_t readLastTimestamp(){return 0;}	// get the lasttimestamp from the end of the last file

	void reset(bool newstream1=false){started=false,lineopen=false;};	//reset the param

private:

	std::vector<unsigned char> getExtraVarBytes(std::vector<GenString> varnames){return std::vector<unsigned char>();};

	std::vector<unsigned char> getHeaderBytes(std::vector<GenString> varnames,uint64_t absolutetimestamp, std::vector<GenString> values){return std::vector<unsigned char>();};

	std::vector<unsigned char> getDataBytes(std::vector<GenString> varnames,uint64_t relativetimestamp, std::vector<GenString> values){return std::vector<unsigned char>();};



	};





#endif



