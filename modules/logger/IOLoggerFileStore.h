#ifndef IOLOGGERFILESTORE_H
#define IOLOGGERFILESTORE_H

#include "../../datastruct/GenString.h"

#include "IOLoggerGen.h"

#include "DataEncoder.h"

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.iologgerfilestore"


#define DEFMAXFILES 5
#define DEFMAXSIZE 1000

#define DEFFOLDER "/log/"
#define DEFFILENAME "logfile"
#define DEFFILEEXT ".json"

////////////////////////////////////////////////
class FileRing{
	unsigned long firstid=0;		// oldest file id
	unsigned long lastid=0;			// newest file id
	unsigned long lastsize=0;

//	unsigned int filesfound=0;

	GenString folder, filename, ext;
	unsigned long filemaxsize;
	unsigned int maxfiles;

public:
	unsigned long getLastSize(){return lastsize;}

public:
	FileRing(GenString folder0, GenString filename0, GenString ext0, unsigned long filemaxsize0, unsigned int maxfiles0):
		folder(folder0),filename(filename0),ext(ext0),filemaxsize(filemaxsize0),maxfiles(maxfiles0){
		// at creation, list the log directory files, see if we have already files starting and ending like ours, take maxmin
		std::vector<FileFS>fslist=CURFS.listFiles(folder,filename);
	//	unsigned int min=0, max=0;
	//	unsigned long maxsize=0;
		firstid=-1;
		for(FileFS fs:fslist){
			GenString fname=fs.name;
			if(endsWith(fname,ext)) {
				unsigned int start=folder.size()+filename.size(), end=fname.size()-ext.size()-start;
				GenString str=fname.substr(start,end);
				unsigned long id=strToUint64(str);
				if(lastid<=id) {lastid=id;lastsize=fs.size;}
				if(firstid>=id) firstid=id;
				println(str);
			}
		}
	if(firstid==(unsigned long)-1) firstid=0;
	/*		else {
			firstid=0;
			if(lastsize>0){filesfound=1;}
		}
	*/	// save the size of last file as start

	}

	GenString getFileFor(unsigned int size){return "";};	// get the filename to which to append this size

	void increaseLastsize(unsigned int size){lastsize+=size;}

	bool fileCanHold(unsigned int size){return (lastsize+size)<filemaxsize;};		// if the file cannot hold it need to be closed
	GenString getFullPath(unsigned long id){return folder+filename+to_string(id)+ext;};
	GenString getCurrentFileFullPath(){return getFullPath(lastid);};	// return full path for current file

	std::vector<GenString> getAllFileNames(unsigned int idmin=0){
		std::vector<GenString> names;
		if(idmin<firstid) idmin=firstid;
		for(unsigned long i=idmin;i<=lastid;i++) names.push_back(filename+to_string(i)+ext);
		return names;};	//return name for current file

	void nextFile(){
		lastid++;
		if((lastid-firstid)>=maxfiles) {// erase first id file
			CURFS.erase(getFullPath(firstid));
			firstid++;
		}
		lastsize=0;
	};
private:

};







////////////////////////////////////////////////
class IOLoggerFileStore
{
	FileRing filering;
	JsonEncoder dataencoder;
	GenMap lasttspervar;

	//	at start scan folder to see what files we have
	// when appending an existing file, close the stream and reopen it at value write
public:
	IOLoggerFileStore ():
		filering(DEFFOLDER, DEFFILENAME,DEFFILEEXT, DEFMAXSIZE-dataencoder.getEndBytes().size(),DEFMAXFILES)  //leave space for endbytes
		{println(RF("Constructor IOLoggerFileStore"));
		};
	bool preloadVarname(GenString varname){return dataencoder.preloadVarname(varname);}

	//void saveToLog
	bool addData(uint64_t tsms,GenString cname, ValType dval){
		println(GenString()+"IOLoggerFileStore::addData: lasttspervar : "+cname+ " -> "+to_string(tsms));
		lasttspervar.setReplace(cname,to_string(tsms));// save name and ts to a map	//it's a multimap but should have a map like set to avoid the preerase;
		bool ns=dataencoder.getNewstream();
		std::vector<unsigned char> dataline=dataencoder.getBytes(tsms,cname,dval);
		std::vector<unsigned char> end=dataencoder.getEndBytes();
		GenString filename=filering.getCurrentFileFullPath();
		unsigned int xtra=0;
		if(ns && filering.getLastSize()>0) xtra=3;
		println(GenString()+"IOLoggerFileStore::addData Last size:"+to_string(filering.getLastSize())+" can hold :"+to_string(filering.fileCanHold(dataline.size()+xtra)));
		if(!filering.fileCanHold(dataline.size()+xtra)) {
			dataline=end;//dataencoder.getEndBytes();
			CURFS.appendToFile(filename,dataline);
			dataencoder.reset();
			filering.nextFile();

			filename=filering.getCurrentFileFullPath();
			dataline=dataencoder.getBytes(tsms,cname,dval);
		} else if(ns && filering.getLastSize()>0) {
			end.push_back(',');
			dataline.insert( dataline.begin(), end.begin(), end.end() );
		}
			//dataline+=dataencoder.getEndBytes();
		// here we are sure to have a file that can hold

		CURFS.appendToFile(filename,dataline);
		filering.increaseLastsize(dataline.size());
		return true;
	};

	uint64_t getLastTSMS(GenString cname){
		GenString str= lasttspervar[cname];
		if(isDigit(str)) return strToUint64(str);
		else return 0;
	}

	GenString getFilesJson(unsigned int id=0){	// get the list of files from oldest to newest as a json
		std::vector<GenString> files=filering.getAllFileNames(id);
		GenString str(RF("["));
		for(unsigned int i=0;i<files.size();i++){
			if(i>0) str+=',';
			str+='\"'+files[i]+'\"';
		}
		str+=']';
		return str;
	}








	////////////////////////////////////////////////////

	void resync(uint64_t diffms){
		//		if(savedts) savedts+=diffms/MILLISFACTOR;
		//		for(std::vector<StreamCell>::iterator it=logdata.begin();it!=logdata.end();it++) it->resync(diffms);	// correct each timestamp
	}

};


#endif



