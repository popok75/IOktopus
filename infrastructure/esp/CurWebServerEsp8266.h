#ifndef CURWEBSERVERESP8266
#define CURWEBSERVERESP8266

#include <map>
#include <ESP8266WebServer.h>
#include "CurFSEsp8266.h"
#undef FTEMPLATE
#define FTEMPLATE ".irom.text.curwebserver"

#define CHUNKSIZE 2048

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

	size_t streamFile(std::string path, std::string contentType,unsigned long start=0,unsigned long stop=0){
		if(CURFS.exists(path)) {
			File file = SPIFFS.open(path.c_str(), RF("r"));
			size_t sent =0;
			if(file){
				println(std::string()+RF("CurWebServerEsp8266::streamFile: sending ")+to_string(start)+" - "+to_string(stop)+", file size: "+to_string(file.size()));
				if(start>file.size()) {file.close();return 0;}
				if(start>0 || (stop>0 && stop<file.size())) sent =streamFilePart(file,contentType,start,stop);
				else sent=  server.streamFile(file, contentType.c_str());
				println(std::string()+RF("CurWebServerEsp8266::streamFile: sent ")+to_string(sent));
				file.close();
			}
			return sent;
		} else println(std::string()+RF("CurWebServerEsp8266::streamFile: path not found :")+path);
		return 0;
	};

	unsigned int streamFilePart(File file, std::string contentType,unsigned long start,unsigned long stop) {
		println(std::string()+RF("CurWebServerEsp8266::streamFilePart: sending ")+to_string(start)+" - "+to_string(stop));
		println(std::string()+RF("CurWebServerEsp8266::streamFilePart: content type ")+contentType);
		if(!stop) stop=file.size();

		int n=CHUNKSIZE;
		unsigned int sent=0,sentall=0;
		char* buff=(char *) malloc(n);
		String nope="";
//		server.send(200, contentType.c_str(), "");
		server.sendContent(String()+RF("HTTP/1.1 200 OK\r\nContent-Type:")+contentType.c_str()+RF("\r\n\r\n")); //send headers
		file.seek(start, SeekSet);
		while (file.position()<stop)
		{
			Serial.println("reading");
			unsigned int nn=n;
			if((sentall+nn)>stop) nn=stop-sentall;
			sent= file.readBytes(buff, nn);
			sentall+=sent;

			server.sendContent_P(buff,sent);

			yield();
		}
		free(buff);

		file.close();
		return sentall;
	}




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


