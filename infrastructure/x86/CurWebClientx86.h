#ifndef CURWEBCLIENTX86
#define CURWEBCLIENTX86




class CWCYieldCallback{
public:
	virtual void yield(){};
};
class CurWebClientx86 {
//	SocketClient *socketClient=0;
//	std::string buffer;
	std::string headers,content;


public:
//	CurWebClientx86();

	bool loadURL(std::string ){return false;};
	bool saveURL(std::string ,std::string, CWCYieldCallback *){return false;};
	bool printURL(std::string ){return false;};

	std::string *getContent(){return &content;}
/*	void open(std::string url);
	unsigned int read(unsigned char *ptr, unsigned int size);
	std::string readString();
	void close();

	void loadAsyncAll(std::string url,void(*cb)(CurWebClientx86*)) ;
*/
};

#endif






