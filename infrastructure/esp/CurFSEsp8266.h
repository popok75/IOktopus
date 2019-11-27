#ifndef CURFSESP8266
#define CURFSESP8266
//#define RF2(x) String(F(x)).c_str()
#include <vector>
#include <FS.h>

//////////////// CurFS

struct FileFS {
	std::string name;
	unsigned int size;
};

struct CurFileFS : public FileFS
{
	File fileptr;
	CurFileFS(std::string name0, unsigned int size0, File fileptr0): fileptr(fileptr0) {name=name0; size=size0;};
};




//#define SPIFFSNOWRITE 1
/*
const char slash[] PROGMEM ={"/"};
const char rn[] PROGMEM ={"\r\n"};
const char rC[] PROGMEM ={"r"};
const char errtxt[] PROGMEM={"readFileBuffer Error: could not open file %s\n"};
 */





class CurFS {
public:
	CurFS(){ SPIFFS.begin();};
	bool exists(std::string path){return SPIFFS.exists(path.c_str());};
	unsigned long fileSize(std::string path){
		File f = SPIFFS.open(path.c_str(), "r");
		if (!f) { println(std::string()+RF("CurFS::fileSize Error: could not open file ")+path.c_str()); return 0; }
		unsigned long sz=f.size();
		f.close();
		return sz;
	};



	void printFiles(){
		// list all files
		String str;
		Dir dir = SPIFFS.openDir(String("/"));
		while (dir.next()) {
			str += dir.fileName();
			str +=  "/";
			str += dir.fileSize();
			str += "\r\n";
		}
		Serial.println(str);
	};

	void eraseAllFiles(){
		// list all files
		String str;
		Dir dir = SPIFFS.openDir(String("/"));
		while (dir.next()) {
			str += dir.fileName();
			str += "\n";
		}
		//Serial.println(str);
		// erase the list
		unsigned i=str.indexOf("\n"),ip=0;
		while (i<str.length()){
			String filename=str.substring(ip,i);
			//	erase(filename);
			Serial.println(RF("Erasing file :")+filename);
			ip=i+1;
			i=str.indexOf("\n",i+1);
		}
	};


	///////////////////////////////
	std::vector<FileFS> listFiles(std::string path0, std::string filenamebase){
		if(path0.substr(path0.length()-1)!="/") path0+="/";
		Dir dir = SPIFFS.openDir(path0.c_str());
		std::vector<FileFS> v;
		filenamebase=path0+filenamebase;
		while (dir.next()) {
			std::string fname=dir.fileName().c_str();
			print(fname+" / "+String(dir.fileSize()).c_str());
			if(fname.substr(0,filenamebase.size())==filenamebase) {
				FileFS ffs;
				ffs.name=fname;
				ffs.size=dir.fileSize();
				v.push_back(ffs);
				Serial.print(RF(" added "));
				Serial.print(ffs.name.c_str());
			}
			Serial.println();

		}
		return v;

	};


	std::vector<FileFS> listFilesByExt(std::string path0, std::string fileext){
		if(path0.substr(path0.length()-1)!="/") path0+="/";
		Dir dir = SPIFFS.openDir(path0.c_str());
		std::vector<FileFS> v;

		while (dir.next()) {
			std::string fname=dir.fileName().c_str();
			print(fname+" / "+String(dir.fileSize()).c_str());
			if(fname.size()>fileext.size() && fname.substr(fname.size()-fileext.size())==fileext) {
				FileFS ffs;
				ffs.name=fname;
				ffs.size=dir.fileSize();
				v.push_back(ffs);
				Serial.print(RF(" added "));
				Serial.print(ffs.name.c_str());
			}
			Serial.println();

		}
		return v;
		return std::vector<FileFS>();};





	bool erase(std::string path) {
#ifndef SPIFFSNOWRITE
		if(SPIFFS.remove(path.c_str())) {println(std::string() + RF("Deleted with success file ")+path);return true;}
		else {println(std::string() + RF("Error deleting file ")+path);return false;}
#else
		return false;
#endif
	};


	unsigned int appendToFile(std::string path, std::vector<unsigned char> vect) {
#ifndef SPIFFSNOWRITE
		File f = SPIFFS.open(path.c_str(), "a");
		if (!f) {println(std::string()+RF("CURFS::appendToFile: Could not appending ")+path);return 0;}
		println(std::string()+RF("CURFS::appendToFile: appending ")+path);

		unsigned int w=f.write(vect.data(), vect.size());
		f.close();
		return w;
#else
		return 0;
#endif
	}


	unsigned int rewriteFile(std::string path, unsigned char *buff, unsigned int size){
#ifndef SPIFFSNOWRITE
		File f = SPIFFS.open(path.c_str(), "w");
		if (!f) {println(std::string()+RF("CURFS::rewriteFile: Could not rewriteFile ")+path);return 0;}
		println(std::string()+RF("CURFS::rewriteFile: rewriting ")+path);

		unsigned int w=f.write(buff, size);
		f.close();
		return w;
#else
		return 0;
#endif
	};


	unsigned int rewriteFile(std::string path, std::vector<unsigned char> vect) {
		return rewriteFile(path,vect.data(),vect.size());
	}

	FileFS* openFile(std::string path){
		File f = SPIFFS.open(path.c_str(), "rb");
		if (!f) return 0;

		return new CurFileFS(path,f.size(),f);
	};

	void closeFile(FileFS* ffs){
		CurFileFS *cffs=(CurFileFS*) ffs;
		cffs->fileptr.close();
		delete cffs;
	};

	unsigned long readFile(unsigned char *r,unsigned long size,FileFS* ffs){
		CurFileFS *cffs=(CurFileFS*) ffs;
		//		long pos = ftell(cffs->fileptr);
		//		println("readFile: reading at pos:"+to_string(pos));
		size_t res= cffs->fileptr.readBytes((char *)r, size);
		//		pos = ftell(cffs->fileptr);
		//		println("readFile: post reading at pos:"+to_string(pos));
		return res;
	};

	void seekFile(unsigned long pos, FileFS* ffs){
		CurFileFS *cffs=(CurFileFS*) ffs;
		cffs->fileptr.seek(pos,SeekSet);
	};

	unsigned char* readFileBuffer(std::string path,size_t &size){
		File f = SPIFFS.open(path.c_str(), "r");
		if (!f) { println(std::string()+RF("readFileBuffer Error: could not open file ")+path.c_str()+"\n"); return 0; }
		unsigned char * buffer = new unsigned char[f.size()];
		size= f.readBytes((char *)buffer, f.size());
		println(std::string()+ RF("reading ")+to_string(size));
		f.close();
		return buffer;
	};

	std::string readFileToString(std::string path,size_t &size){
		File f = SPIFFS.open(path.c_str(), "r");
		if (!f) { println(std::string()+RF("readFileToString Error: could not open file '")+path.c_str()+"'\n"); return std::string(); }
		unsigned char * buffer = new unsigned char[f.size()];
		size= f.readBytes((char *)buffer, f.size());
		println(std::string()+ RF("reading ")+to_string(size));
		f.close();
		std::string str((const char *)buffer);
		delete buffer;
		return str;
	};

	void printFile(std::string path,size_t &size, int blocksize=128){
		File f = SPIFFS.open(path.c_str(), "r");
		if (!f) { println(std::string()+RF("printFile Error: could not open file ")+path.c_str()+"\n"); return ; }
		int sz=f.size();
		size=0;
		unsigned char * buffer = new unsigned char[blocksize+1];
		while(sz>0){
			if(sz<blocksize) blocksize=sz;
			size= f.readBytes((char *)buffer, blocksize);
			buffer[blocksize]=0;
			Serial.print(String((char *)buffer));
			sz-=blocksize;
		}
		f.close();
		return;
	};
	/*
	void printFileContent(std::string path){
			File dataFile = SPIFFS.open(path.c_str(), "r");   //Ouverture fichier pour le lire
			Serial.println("Lecture du fichier en cours:");
			if(dataFile){
				//Affichage des données du fichier
				for(int i=0;i<dataFile.size();i++)
					Serial.print((char)dataFile.read());    //Read file
			} else Serial.println("Could not open file for read");
			dataFile.close();
		}*/
} CURFS; //684 static mem, i guess its spiffs.begin()


/* SPIFFS.begin : 700 bytes
#include <FS.h>
void setup() {
    SPIFFS.begin();
}
void loop() { }
 */


#endif
