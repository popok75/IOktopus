#ifndef CURWEBSERVERESP8266
#define CURWEBSERVERESP8266

#include <map>
#include <ESP8266WebServer.h>
#include "CurFSEsp8266.h"
#undef FTEMPLATE
#define FTEMPLATE ".irom.text.curwebserver"


class CurWebServerEsp8266 {
public:
	int port;
	ESP8266WebServer server;

	CurWebServerEsp8266(unsigned int port0=80): port(port0), server(port0){};

	void collectHeaders(const char **headers,size_t length){server.collectHeaders(headers,length);}

	void begin(){server.begin();};
	static void onFound(){

	}
	void on(std::string path, HTTPMethod getpost, void (*func)()){
		server.on(path.c_str(), getpost, func);
	};
	void onNotFound(void (*func)()){server.onNotFound(func);};
	void handleClient(){server.handleClient();};//do nothing
	void sendHeader( std::string header, std::string value){server.sendHeader(header.c_str(), value.c_str());};

	void send(int status, std::string type, std::string message){server.send(status, type.c_str(), message.c_str());};
	void sendContent(std::string message){server.sendContent(message.c_str());};
	void setContentLength(const size_t contentLength){server.setContentLength(contentLength);};
	size_t streamFile(std::string path, std::string contentType){
		if(CURFS.exists(path)) {
			File file = SPIFFS.open(path.c_str(), RF("r"));
			size_t sent =0;
			if(file){
				sent=  server.streamFile(file, contentType.c_str());
				println(std::string()+RF("sent ")+to_string(sent));
				file.close();
			}
			return sent;
		}
		return 0;
	};
	std::string uri(){return std::string(server.uri().c_str());};
	std::map<std::string,std::string> getArguments(){
		std::map<std::string,std::string> ret;
		int argn=server.args();
		for(int i=0;i<argn;i++) ret[server.argName(i).c_str()]=server.arg(i).c_str();
		return ret;
	};
	bool hasArg(std::string arg){return server.hasArg(arg.c_str());}
	bool hasHeader(std::string arg){return server.hasHeader(arg.c_str());}
	std::string header(std::string arg){return std::string(server.header(arg.c_str()).c_str());}

};

#endif


