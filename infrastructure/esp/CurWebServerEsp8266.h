#ifndef CURWEBSERVERESP8266
#define CURWEBSERVERESP8266

#define USEMDNS true

#include <map>
#include <ESP8266WebServer.h>
#if USEMDNS
#include <ESP8266mDNS.h>
#endif
#include "CurFSEsp8266.h"

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.curwebserver"

#define CHUNKSIZE 2048

class CurWebServerEsp8266 {
public:
	int port;
	ESP8266WebServer server;
	GenString mdnsname;
	bool mdnsstarted=false;

	CurWebServerEsp8266(unsigned int port0=80): port(port0), server(port0){};

	void collectHeaders(const char **headers,size_t length){server.collectHeaders(headers,length);}

	void setMdnsName(GenString newmdnsname){mdnsname=newmdnsname;}


	void begin(){
#if USEMDNS
		if(!mdnsname.empty()) {
			Serial.println(String()+"CurWebServerEsp8266 starting mdns with name"+mdnsname.c_str());
			mdnsstarted=MDNS.begin(mdnsname.c_str());
		}
#endif

		Serial.println("CurWebServerEsp8266 starting server");
		server.begin();

#if USEMDNS
		if(mdnsstarted) MDNS.addService("http", "tcp", 80);
#endif
	};


	static void onFound(){}

	void on(std::string path, HTTPMethod getpost, void (*func)()){
		server.on(path.c_str(), getpost, func);
	};
	void on(std::string path, HTTPMethod getpost, void (*func)(),void (*func2)()){
			server.on(path.c_str(), getpost, func,func2);
		};
	void onNotFound(void (*func)()){server.onNotFound(func);};
	void handleClient(){
#if USEMDNS
		if(mdnsstarted) MDNS.update();
#endif
		server.handleClient();
	};
	void sendHeader( std::string header, std::string value){server.sendHeader(header.c_str(), value.c_str());};
	void send(int status){server.send(status);};
	void send(int status, std::string type, std::string message){server.send(status, type.c_str(), message.c_str());};
	void sendContent(std::string message){
		server.sendContent(message.c_str());
		//	if(message.size()==0) server.client().stop();	//seems useless
		//	yield();	//seems useless too
	};
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
			//		Serial.println("reading");
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

	File fsUploadFile;
	bool uploadFile(){
		HTTPUpload& upload = server.upload();
		println(GenString()+"handleFileUpload "+to_string(upload.status)+" "+to_string(UPLOAD_FILE_START));
		if(upload.status == UPLOAD_FILE_START){
			String filename = upload.filename;
			if(!filename.startsWith("/")) filename = "/"+filename;
			Serial.print("handleFileUpload name: "); Serial.println(filename);
			fsUploadFile = SPIFFS.open(filename, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
			filename = String();
		} else if(upload.status == UPLOAD_FILE_WRITE){
			if(fsUploadFile)
				fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
		} else if(upload.status == UPLOAD_FILE_END){
			if(fsUploadFile) {                                    // If the file was successfully created
				fsUploadFile.close();                               // Close the file again
				Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
				//  server.sendHeader("Location","/success.html");      // Redirect the client to the success page
				//  server.send(303);
			} else {
				return false;

			}
		}
		return true;
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


