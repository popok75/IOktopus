#ifndef CURFSx86_H
#define CURFSx86_H

#include <string>
#include <vector>

struct FileFS {
public:
	std::string name;
	unsigned int size;

};
struct CurFileFS : public FileFS
{
	FILE *fileptr;
	CurFileFS(std::string name0, unsigned int size0, FILE *fileptr0): fileptr(fileptr0) {name=name0; size=size0;};
};


class CurFS {
public:
	std::string programpath = ".\\";//"C:\\Users\\Pok\\Documents\\Arduino\\IOServer\\data\\";
	CurFS(){};
	~CurFS(){};

	std::string pathToFinal(std::string path);

	unsigned long fileSize(std::string path) ;

	bool exists(std::string path) ;

	std::vector<FileFS> listFilesByExt(std::string path0, std::string fileext);
	std::vector<FileFS> listFiles(std::string path0, std::string filenamebase);
	void printFiles(std::string path0="");

	bool erase(std::string path) ;
	unsigned int appendToFile(std::string path, std::vector<unsigned char> &vect);
	unsigned int rewriteFile(std::string path, std::vector<unsigned char> &vect) ;
	unsigned int rewriteFile(std::string path, unsigned char *buff, unsigned int size) ;

	unsigned char* readFileBuffer(std::string path,size_t &size);
	std::string readFileToString(std::string path,size_t &size);
	void printFile(std::string path,size_t &size, unsigned int blocksize=128){}
	template <typename... ParamTypes>
	//	void setTimeOut(unsigned int milliseconds, bool repeat, bool **cancel, void (func)(ParamTypes...), ParamTypes... parames)
	bool readByChunk( std::string path, long chunksize,int backward, bool (func)(unsigned char*,ParamTypes...), ParamTypes... parames);

	FileFS* openFile(std::string path);
	void closeFile(FileFS* ffs);
	unsigned long readFile(unsigned char *r,unsigned long size,FileFS* ffs);
	void seekFile(unsigned long pos, FileFS* ffs);

}; // static instance of CURFS



#endif
