#ifndef CURWEBSERVERX86
#define CURWEBSERVERX86



#include <map>


class httpconnHandler;
class httpserverHandler;

#define HTTP_GET 1
#define CONTENT_LENGTH_UNKNOWN 1

class CurWebServerx86 {
	bool started=false;
	int port;
	httpserverHandler *ppserver;

	std::string programpath="";

public:
	std::string lasturi;
	httpconnHandler *lastreq;

	CurWebServerx86(unsigned int port0=80);

	void collectHeaders(const char **headers,size_t length){} //all headers are always collected

	void begin();
	void run();

	std::map<std::string, void (*)()> callbacks;
	void onHttp(std::string uri) ; // called by the httpsocket to notify server

	void on(std::string path, int getpost, void (*func)()){callbacks[path]=func;};
	void onNotFound(void (*func)()){callbacks["notfound"]=func;};

	bool hasHeader(std::string name);       // check if header exists
	bool hasArg(std::string arg){return false;} //??
	std::string header(std::string name);      // get request header value by name
	std::map<std::string,std::string> getArguments();
	std::string uri(){return lasturi;};

	void setContentLength(int ){};//dummy function -> require webserver.cpp modification to modify contentLength
	void sendContent( std::string header);
	void sendHeader( std::string header, std::string value);
	void send(int status, std::string type, std::string message);;
	size_t streamFile(std::string filename, std::string contentType);	// send a file to current req


	void handleClient();	// call mono yield function




};


#endif






