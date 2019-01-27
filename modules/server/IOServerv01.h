#ifndef IOSERVERV01_H
#define IOSERVERV01_H

#include "../../infrastructure/CompatFS.h"
#include "../../infrastructure/CompatNet.h"
//#include "../../infrastructure/CompatTicker.h"
#include "../../infrastructure/SyncedClock.h"

#include "../../infrastructure/CompatCrash.h"

#include "IOServerGen.h"

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.ioserverv01"

//#define MULTIPARTLOG true

#define MAXHTTPCHUNK 2048

class IOServerv01;

IOServerv01 *statinst;	// crappy and fast way to manage the callback, should be improved

GenString getContentType(GenString filename, bool download);
void startreply(IOServerv01 *server);
/*
std::vector<unsigned char *> reserved ;
unsigned long minmem=12000;
bool reserveMeminit=false;
void reserveMem(){
	if(reserveMeminit) return;
	reserveMeminit=true;
#ifdef ESP8266
			print("IOServerv01::reserveMem : free memory before :");	println(ESP.getFreeHeap(),DEC);

			while(ESP.getFreeHeap()>minmem){unsigned char *res=new unsigned char [1024]; reserved.push_back(res);}

			print("IOServerv01::reserveMem : free memory after:");	println(ESP.getFreeHeap(),DEC);
#endif
	};

unsigned int totalfake=55000;
void fakeLog(MultipartStringEvent *se){
		se->totalsize=totalfake;
		se->str="";
		unsigned int chunknumber=totalfake/se->maxsize, sz=se->maxsize;
		if(se->index==chunknumber){se->tobecontinued=false;sz=totalfake-chunknumber*se->maxsize;}
		else se->tobecontinued=true;
		char c='0'+se->index%10;
		for(unsigned int i=0;i<sz;i++){se->str+=c;}
	};
 */
class IOServerv01: public IOServerGen
{
public:
	bool chunked=true;

	IOServerv01 (unsigned int httpport=80): IOServerGen(httpport){
		println(RF("Constructor IOServerv01"));
	};
	IOServerv01 (GenString name, unsigned int httpport=80): IOServerGen(name,httpport){println(RF("Constructor IOServerv01"));};

	void setIOData(IODataGen *myiodata1){myiodata=myiodata1;}

	bool handleRequest(GenString path){
		println(RF("----------------->"));
#ifdef ESP8266
		print(RF("IOServerv01::handleRequest : free memory :"));	println(ESP.getFreeHeap(),DEC);
#endif
		if(path.length()>1 && path[path.length()-1]=='/') path.erase(path.length()-1);		//		print("IOServer::handleFileRead: mem 0 :");println(ESP.getFreeHeap(),DEC);
		println(RF("IOServer::handleFileRead : serving uri : ")+path);
		/*		if(path=="/ping") { // not implemented yet
			server.send(200, "image/png","pong");
			return true;
		}*/
		GenString contentType;
		if(path==RF("/")) {path=RF("/index.html");contentType=RF("text/html");}
		else contentType=getContentType(path, server.hasArg(RF("download")));//"text/plain";

		/*	 if(server.hasHeader("x-Date")) {	// resync server time from client // not implemented yet
			std::string str=server.header("x-Date");
			println("found resync header: "+str);
			CLOCK.resync(stoull(str)/1000);
			CLOCK.clean=true;
		}*/
		//		 println(GenString()+"IOServerv01::handleRequest: Accept:"+to_string((unsigned int)server.hasHeader("Accept")));		 //		 println(GenString()+"IOServerv01::handleRequest: Accept:"+server.header("Accept"));		 //		 println(GenString()+"IOServerv01::handleRequest: x-BootTime:"+to_string((unsigned int)server.hasHeader("x-BootTime")));
		if(server.hasHeader("x-BootTime")){
			//	 println("IOServerv01::handleRequest: x-BootTime true");
			server.sendHeader("x-BootTime", to_string(CLOCK32.getBoottime()));
		}

		if(path==RF("/eraseCrashDump")) {CurSaveCrash.clear();server.send(200, RF("application/json"),"CrashDump file erased\n");return true;}

		if(path==RF("/data") && myiodata) {
			// GenString message=myiodata->getAsJson();
			// case of sync events
			StringEvent strev=StringEvent();
			emit(RF("getAsJson"),&strev);				// get the data by sync event rather than through getAsJson
			//			server.send(200, "application/json",strev.str.c_str(),strev.str.size());
			server.send(200, RF("application/json"),strev.str);	//is the server duplicating the string (no reason)
			return true;
		}

		if(path==RF("/log")) {
			serveLog();
			return true;
		}

		unsigned int start=0, end=0;
		GenString rangestr=getArgument(RF("range"));
		if(!rangestr.empty()){
			unsigned int i=rangestr.find('-');
			if(i<rangestr.size()){
				GenString startstr=rangestr.substr(0,i);
				GenString endstr=rangestr.substr(i+1,rangestr.size());
				if(isDigit(startstr)) start=strToUint64(startstr);
				if(isDigit(endstr)) end=strToUint64(endstr);
			}
		}


		if(startsWith(path,"/log/")) {
			bool b=(server.streamFile(path, contentType,start,end)>0);
			if(b) println(GenString()+RF("Streamed : ")+path+RF(" ")+ contentType +RF(" : 1"));
			else println(GenString()+RF("Streamed : ")+path+RF(" ")+ contentType +RF(" : 0"));
			return b;
		}


		if (CURFS.exists(path+RF(".gz"))) {
			path += RF(".gz");
			println(GenString()+RF("Added gz to path: ")+path);
			//	server.sendHeader("Content-Encoding", "gzip");	// on esp8266 it bugs if we add content-type (it is added later on)
		} else {println(GenString()+RF("Did not add gz to path: ")+path);}


		/*	if(path=="/favicon.ico") {
			path="/favicon.png";
			contentType="image/jpeg";
		}
		 */
		bool b=(server.streamFile(path, contentType,start,end)>0);
	//	bool b=(server.streamFile(path, contentType)>0);
		if(b) println(GenString()+RF("Streamed : ")+path+RF(" ")+ contentType +RF(" : 1"));
		else println(GenString()+RF("Streamed : ")+path+RF(" ")+ contentType +RF(" : 0"));

		return b;
	}


	GenString getArgument(GenString argname){
		std::map<std::string,std::string> args=server.getArguments();
		for(auto it : args) {
			println(GenString()+RF("Found argument :")+it.first+":"+it.second);
			if(it.first==argname) {
				return it.second;
			}
		}
		return "";
	}


	void serveLog(){

		// we need a way to serve both 0.16 and 0.20 logs :
		// - 0.16 will give a n chunks of X bytes of json data that can start from ts>0
		// - 0.2 will serve a list of log files
		// 		- when data files are fetched a range request streamer would be a good idea (from http header or url argument)

		GenString tsstr=getArgument(RF("fromTS")),idstr=getArgument(RF("fromID"));
		uint64_t ts=0,id=0;
		if(!tsstr.empty() && isDigit(tsstr)) ts=strToUint64(tsstr)*1000;
		if(!tsstr.empty() && isDigit(idstr)) id=strToUint64(idstr);



		NamedStringMapEvent strev=NamedStringMapEvent("",{{RF("max"),to_string(MAXHTTPCHUNK)}});
		if(ts) strev.values.set(RF("ts"),to_string(ts));
		if(id) strev.values.set(RF("id"),to_string(id));

		emit(RF("getLogJson"),&strev);

		unsigned int totalsize=0;
		GenString totstr=strev.values.get(RF("total"));
		if(isDigit(totstr)) totalsize=strToUint64(totstr);
		if(chunked || !totalsize){	// for correct chunked transfer mode must add the size and \n\r : https://en.wikipedia.org/wiki/Chunked_transfer_encoding // or else ERR_INCOMPLETE_CHUNKED_ENCODING
			server.setContentLength(CONTENT_LENGTH_UNKNOWN);
		} else {
			server.setContentLength(totalsize);
		}
		//unsigned long ttsz=strev.ename.size();
		server.send (200, RF("text/html"), strev.ename);
		while (strev.values.has(RF("tobecontinued"))) {
			emit(RF("getLogJson"),&strev);
			server.sendContent(strev.ename);
#ifdef ESP8266
			print(RF("IOServerv01::handleRequest : free memory while sending log :"));	println(ESP.getFreeHeap(),DEC);
#endif
		}
		/*
		//		 StringEvent strev=StringEvent();
		//		 emit("getLogJson",&strev);				// get the data by sync event rather than through getAsJson
		//		 server.send(200, "application/json",strev.str);
				 std::map<std::string,std::string> args=server.getArguments();
				 uint64_t ts=0;
				 for(auto it : args) {
					 println(GenString()+"Found argument :"+it.first+":"+it.second);
					 if(it.first==RF("from") && isDigit(it.second)) {
						 ts=strToUint64(it.second)*1000;
						 break;}
				 }
				 if(!MULTIPARTLOG){	//this part is not functional anymore
					 // GenString message=myiodata->getAsJson();
					 // case of sync events
					 StringEvent strev=StringEvent();
					 emit(RF("getLogJson"),&strev);				// get the data by sync event rather than through getAsJson
					 //			server.send(200, "application/json",strev.str.c_str(),strev.str.size());
					 server.send(200, RF("application/json"),strev.str);	//is the server duplicating the string (no reason)

				 } else {
					 StringMapEvent strev=StringMapEvent(ts,2048);
					 emit(RF("getLogJson"),&strev);

					 if(chunked){	// for correct chunked transfer mode must add the size and \n\r : https://en.wikipedia.org/wiki/Chunked_transfer_encoding // or else ERR_INCOMPLETE_CHUNKED_ENCODING
						 server.setContentLength(CONTENT_LENGTH_UNKNOWN);
					 } else {
						 server.setContentLength(strev.totalsize);
					 }
					 unsigned long ttsz=strev.str.size();
					 server.send (200, RF("text/html"), strev.str);
				//	 println(GenString()+"IOServerv01::handleRequest : Server sending log "+to_string(strev.index)+" :"+strev.str);
					 strev.index++;

					 while (strev.tobecontinued) {
						 emit(RF("getLogJson"),&strev);
					//	 fakeLog(&strev);
						 server.sendContent(strev.str);
						 ttsz+=strev.str.size();
					//	 println(GenString()+"IOServerv01::handleRequest : Server sending log "+to_string(strev.index)+" :"+strev.str);
						 strev.index++;
					//	 delay (5000);
		//				 server.client().wait_until_sent(100);
		//				 println(GenString()+"IOServerv01::handleRequest : Server sending log wait taken "+);
					//	 delay(100);
					//	 yield();	// give bit of time to esp to send the packet
	#ifdef ESP8266
				print(RF("IOServerv01::handleRequest : free memory while sending log :"));	println(ESP.getFreeHeap(),DEC);
	#endif
					 }
				//	 uint32_t ts1=millis();
			//		 println("IOServerv01::handleRequest : total size declared "+ to_string(strev.totalsize)+" counted"+to_string(ttsz));
		/ *			 if(strev.totalsize!=ttsz){
						 println("IOServerv01::handleRequest : total sizes differ : declared "+ to_string(strev.totalsize)+" counted"+to_string(ttsz));

						 MultipartStringEvent strev2=MultipartStringEvent(512);
						 emit("getLogJson",&strev2);
						 ttsz= strev2.str.size();
						 strev2.index++;
						 while (strev2.tobecontinued) {
							 emit("getLogJson",&strev2);
							 ttsz+=strev2.str.size();
							 strev2.index++;
						 }
						 println("IOServerv01::handleRequest : total size2 declared  "+ to_string(strev.totalsize)+" counted"+to_string(ttsz));

					 }* /
				//	 if(chunked) server.sendContent("");
				 }*/
	}

	void start(){
		statinst=this;

		server.on(RF("/"), HTTP_GET, []() {
			println(GenString()+RF("HTTP request "));
			if (!statinst->handleRequest(statinst->server.uri()))
				statinst->server.send(404, RF("text/plain"), statinst->server.uri()+RF(" : File Not Found"));
		});

		server.onNotFound([]() {                          // Handle when user requests a file that does not exist
			if (!statinst->handleRequest(statinst->server.uri()))
				statinst->server.send(404, RF("text/plain"), statinst->server.uri()+RF(" : File Not Found"));
		});
		//	webSocket.begin();                                // start webSocket server
		//	webSocket.onEvent(webSocketEvent);                // callback function

		const char * headerkeys[] = {"x-BootTime","Accept"} ;
		size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
		//ask server to track these headers
		server.collectHeaders(headerkeys, headerkeyssize );

		server.begin();
		println(RF("HTTP server started"));
		/*	if (MDNS.begin("myesp")) {  //Start mDNS
				println("MDNS started");
				MDNS.addService("http", "tcp", 80);
			}
		 */

		//	webSocket.sendTXT(socketNumber, "wpMeter,Arduino," + temp_str + ",1");
		//	println("websocket server started");

	};
	/*
	void reply(){
						server.send(200, "application/json",datajson);
						ticker.detach();waitingreply=false;
	};
	 */
private:
};
#undef FTEMPLATE
#define FTEMPLATE ".irom.text.getcontenttype"
std::string  getContentType( std::string  filename, bool download) {	//exist somewhere (in esp8266webserver.h) should not be duplicated
	if (download)      return RF("application/octet-stream");
	else if (endsWith(filename,RF(".htm"))) return RF("text/html");
	else if (endsWith(filename,RF(".html")))return RF("text/html");
	else if (endsWith(filename,RF(".css"))) return RF("text/css");
	else if (endsWith(filename,RF(".js")))  return RF("application/javascript");
	else if (endsWith(filename,RF(".png"))) return RF("image/png");
	else if (endsWith(filename,RF(".gif"))) return RF("image/gif");
	else if (endsWith(filename,RF(".jpg"))) return RF("image/jpeg");
	else if (endsWith(filename,RF(".ico"))) return RF("image/x-icon");
	else if (endsWith(filename,RF(".xml"))) return RF("text/xml");
	else if (endsWith(filename,RF(".pdf"))) return RF("application/x-pdf");
	else if (endsWith(filename,RF(".zip"))) return RF("application/x-zip");
	else if (endsWith(filename,RF(".gz")))  return RF("application/x-gzip");
	else if (endsWith(filename,RF(".svg"))) return RF("image/svg+xml");
	return RF("text/plain");
}


#endif
