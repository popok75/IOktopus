#ifndef IOSERVERV01_H
#define IOSERVERV01_H
/*

 */
#include "../../infrastructure/CompatFS.h"
#include "../../infrastructure/CompatNet.h"
//#include "../../infrastructure/CompatTicker.h"
#include "../../infrastructure/SyncedClock.h"

#include "../../infrastructure/CompatCrash.h"
#include "../../infrastructure/ErrorLogFile.h"



#include "IOServerGen.h"

//#include "../../httpdownload.h"



#include "../../bootstrap.h"


#undef FTEMPLATE
#define FTEMPLATE ".irom.text.ioserverv01"

#define SERVERADMINPASSWD_KEYWORD "server-admin-password"
#define SERVERPUBLICWRITE_KEYWORD "server-public-write"
#define SERVERPRIVATEREAD_KEYWORD "server-private-read"
#define SERVERMDNSNAME_KEYWORD "server-mdns-name"
#define SERVERDEFAULTMDNSNAME "ioktopus"
//#define MULTIPARTLOG true

#define MAXHTTPCHUNK 2048

#define SERVERBACKUPPAGEPRE1 "<fieldset id='linkgroup'	style='position: absolute; right: 2em; top: 1em; border: 1px dotted; font-size: 80%;'>switch to faster local ip: <a href='http://"
#define SERVERBACKUPPAGEPRE2 "'>http://"
#define SERVERBACKUPPAGEPRE3 "/</a></fieldset>"

#define SERVERBACKUPPAGE_WIFI "<form id='wifiform' action='/setWifi'><b>Set wifi </b><br> ssid <input type='text' name='ssid' size=25> password <input type='password' name='ssidpassword' size=25> <br> mdns name (optional)<input type='text' name='mdnsname' size=15 placeholder='ioktopus'> <input type='submit' value='submit'></form>"
#define SERVERBACKUPPAGE_POPULATE "<form id='bform' action='/populateFiles'>from file list url: <input type='text' name='filelisturl' size=80> <br><input type='submit' value='Submit'> <br>or file list content: <textarea form='bform' rows=10 name='content' style='width:100%;white-space: pre;'></textarea></form>"




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
	GenMap *config=0;
	Ticker serverticker;
	bool serverpopulate=false;
	bool serverbusy=false;
	GenString filelisturl, filelistcontent;

	IOServerv01 (unsigned int httpport=80, GenMap* config0=0): IOServerGen(httpport){
		println(RF("Constructor IOServerv01"));
		config=config0;
	};
	IOServerv01 (GenString name, unsigned int httpport=80, GenMap* config0=0): IOServerGen(name,httpport){println(RF("Constructor IOServerv01"));config=config0;};
	IOServerv01 (GenString name, GenMap* config0=0): IOServerGen(name,80){println(RF("Constructor IOServerv01"));config=config0;};

	void setIOData(IODataGen *myiodata1){myiodata=myiodata1;}

	static void rebootEsp(){	//should be moved to  somethingCompat.h
#ifdef ESP8266BUILD
		ESP.restart();
		//	ESP.reset();
		//	WiFi.forceSleepBegin(); wdt_reset(); ESP.restart(); while(1)wdt_reset();
#endif

#ifdef x86BUILD
		exit(1);
#endif
	}


	void downloadFiles(GenString filelisturl1, GenString content=""){
		class serverYieldCallback:public CWCYieldCallback {
			IOServerv01 *server;
		public:
			serverYieldCallback(IOServerv01 *server0):server(server0){};
			void yield(){
				//				println("IOServerv01:IOServer yield callback");
				if(server) {
					server->yield();
					//delay(20);
				}}
		} yieldcb(this);
		println("IOServerv01:downloadFiles:: pre downloadFile");
		downloadFile(filelisturl1, content, &yieldcb);
		println("IOServerv01:downloadFiles:: post downloadFile");
		//	testDownload();
		//	servertest=true;
	}


	void yield(){
		//	println("IOServer yield regular function");

		IOServerGen::yield();

		if(serverpopulate && !serverbusy) {
			serverbusy=true;
			//			println("downloadFiles from yield IOServerv01");

			downloadFiles(filelisturl,filelistcontent);
			filelisturl="";filelistcontent=""; //erase the content to save some memory
			serverpopulate=false;
			serverbusy=false;
		}
	}


	bool passwordMatches(bool*passwordprovided=0){
		GenString pwd=getArgument(RF("password"));
		GenString adminpwd=config->get(RF(SERVERADMINPASSWD_KEYWORD));
		if(passwordprovided) *passwordprovided=!pwd.empty();
		return (pwd==adminpwd);
	};


	bool writePermitted(GenString path, bool*passwordprovided=0){	// check if path is in public write or else if password matches
		if(!config) return false;
		GenString publicwrite=config->get(RF(SERVERPUBLICWRITE_KEYWORD));
		unsigned int pos=publicwrite.find(path);
		if(pos<publicwrite.size()) return true;

		return passwordMatches(passwordprovided);
	};


	bool readPermitted(GenString path, bool*passwordprovided=0){	// check if path is in private read and if password matches
		if(!config) return true;
		GenString publicwrite=config->get(RF(SERVERPRIVATEREAD_KEYWORD));
		unsigned int pos=publicwrite.find(path);
		if(pos<publicwrite.size()) return passwordMatches(passwordprovided);
		return true;
	};


	GenString makeContentType(GenString &path){
		GenString contentType;
		if(path==RF("/")) {contentType=RF("text/html");}
		else contentType=getContentType(path, server.hasArg(RF("download")));//"text/plain";
		return contentType;
	}

	GenString getArgument(GenString argname){
		std::map<std::string,std::string> args=server.getArguments();
		for(auto it : args) {
			//		println(GenString()+RF("Found argument :")+it.first+":"+it.second);
			if(it.first==argname) {
				return it.second;
			}
		}
		return "";
	}


	void printArguments(){
		std::map<std::string,std::string> args=server.getArguments();
		for(auto it : args) {
//			uint64_t ts=0;
			println(GenString()+"Found argument : '"+it.first+"'='"+it.second+"'");
	/*		if(it.first==RF("from") && isDigit(it.second)) {
				ts=strToUint64(it.second)*1000;
				break;
			}
	*/	}
	}

	bool handleRequest(GenString path){
		println(RF("----------------->"));
#ifdef ESP8266
		print(RF("IOServerv01::handleRequest : free memory :"));	println(ESP.getFreeHeap(),DEC);
#endif
		// if last char is '/', erase it
		if(path.length()>1 && path[path.length()-1]=='/') path.erase(path.length()-1);		//		print("IOServer::handleFileRead: mem 0 :");println(ESP.getFreeHeap(),DEC);

		println(RF("IOServer::handleFileRead : serving uri : '")+path+"'");
		printArguments();



		// client request boot timestamp
		if(server.hasHeader(RF("x-BootTime"))) server.sendHeader(RF("x-BootTime"), to_string(CLOCK32.getBoottime()));//	 println("IOServerv01::handleRequest: x-BootTime true");

		if(path==RF("/setWifi")) serveSetWifi();

		if(path==RF("/populateFiles"))	return servePopulate();

		if(path==RF("/eraseCrashDump")) serveEraseCrashDump(path);
		if(path==RF("/eraseErrorLog")) serveEraseErrorLog(path);

		if(path==RF("/reboot")) return serveReboot(path);

		if(startsWith(path,"/filewrite")) serveFileWrite(path);
		if(startsWith(path,"/fileerase")) return serveFileErase(path);

		bool root=false;
		if(path==RF("/")) {path=RF("/index.html");root=true;}
		GenString contentType=makeContentType(path);
		if (CURFS.exists(path+RF(".gz"))) path += RF(".gz");//	server.sendHeader("Content-Encoding", "gzip");	// on esp8266 it bugs if we add content-type (it is added later on by the lib)

		bool pp;//check file permissions
		if(!readPermitted(path,&pp)) return servePermissionDenied(pp);

		if(path==RF("/data")) return serveData();
		if(startsWith(path,"/data/")) return serveDataNode(path);

		if(path==RF("/log")) {serveLog();return true;}

		unsigned int start=0, end=0;
		getRange(start,end);

		if(startsWith(path,"/log/")) return streamLogFile(path,start,end);


		if((root && !CURFS.exists(path)) || path==RF("/rescue")) {
			//		println(GenString()+"path not found :"+path+", serving rescue page");
			return serveRescuePage();	//either index.html or index.html.gz
		}


		bool b=(server.streamFile(path, contentType,start,end)>0);
		if(b) println(GenString()+RF("Streamed : ")+path+RF(" ")+ contentType +RF(" : 1"));
		else println(GenString()+RF("Streamed : ")+path+RF(" ")+ contentType +RF(" : 0"));

		return b;

		/*
		if(path=="/ping") { // not implemented yet
			server.send(200, "image/png","pong");
			return true;
		}
				  if(server.hasHeader("x-Date")) {	// resync server time from client // not implemented yet
					std::string str=server.header("x-Date");
					println("found resync header: "+str);
					CLOCK.resync(stoull(str)/1000);
					CLOCK.clean=true;
				}
				//		 println(GenString()+"IOServerv01::handleRequest: Accept:"+to_string((unsigned int)server.hasHeader("Accept")));

				 	if(path=="/favicon.ico") {
					path="/favicon.png";
					contentType="image/jpeg";
				}
		 */
	}






	bool getRange(unsigned int &start, unsigned int &end){
		GenString rangestr=getArgument(RF("range"));
		if(!rangestr.empty()){
			unsigned int i=rangestr.find('-');
			if(i<rangestr.size()){
				GenString startstr=rangestr.substr(0,i);
				GenString endstr=rangestr.substr(i+1,rangestr.size());
				if(isDigit(startstr)) start=strToUint64(startstr);
				if(isDigit(endstr)) end=strToUint64(endstr);
				return true;
			}
		}
		return false;
	}







	bool serveEraseCrashDump(GenString &path){
		bool bec=false;
		if(writePermitted(path,&bec)) {CurSaveCrash.clear();server.send(200, RF("application/json"),RF("CrashDump file erased\n"));return true;}
		else {
			if(bec) {server.send(403, RF("text/html"),RF("provided password is incorrect\n"));return true;}
			else {server.send(403, RF("text/html"),path+RF(" is protected by password\n"));return true;}
		}
	}


	bool serveEraseErrorLog(GenString &path){bool bec=false;
	if(writePermitted(path,&bec)){eraseErrorLogFile();server.send(200, RF("application/json"),RF("ErrorLog file erased\n"));return true;}
	else {
		if(bec) {server.send(403, RF("text/html"),RF("provided password is incorrect\n"));return true;}
		else {server.send(403, RF("text/html"),path+RF(" is protected by password\n"));return true;}
	}
	}


	bool serveReboot(GenString &path){
		bool bec=false;
		if(writePermitted(path,&bec)) {serverticker.once_ms(2000, rebootEsp);server.send(200, RF("text/html"),RF("<meta http-equiv='refresh' content='10;url=/' />Rebooting in 5sec..bye\n"));return true;}
		else {
			if(bec) {server.send(403, RF("text/html"),RF("provided password is incorrect"));return true;}
			else {server.send(403, RF("text/html"),path+RF(" is protected by password\n"));return true;}
		}
	}



	bool serveFileWrite(GenString &path){
		bool b=1;
		GenString p0=getArgument(RF("path"));
		GenString cont=getArgument(RF("content"));
		println(GenString()+RF("Filewrite : ")+path+RF(" ")+ p0 +RF(" :")+cont);
		if(startsWith(p0,"http://")) {//remove http and host
			int pos=p0.find("/",7);
			if(pos>=0) p0=p0.substr(pos);
		}
		if(config) {
			if(writePermitted(p0)){//check permissions
				CURFS.rewriteFile(p0,(unsigned char *)cont.c_str(),cont.size());//write file
				server.send(200, RF("text/html"),RF("File ")+p0+ RF(" updated\n"));
			} else server.send(403, RF("text/html"),RF("File write protected and password didn't match !\n"));
		} else server.send(403, RF("text/html"),RF("No configuration available to server !\n"));

		return b;
	}


	bool serveFileErase(GenString &path){
		bool b=1;
		GenString p0=getArgument(RF("path"));
		println(GenString()+RF("Fileerase : ")+path+RF(" ")+ p0);
		if(startsWith(p0,"http://")) {//remove http and host
			int pos=p0.find("/",7);
			if(pos>=0) p0=p0.substr(pos);
		}
		if(config) {//check permissions
			if(writePermitted(p0)){
				CURFS.erase(p0);//write file
				server.send(200, RF("text/html"),RF("File ")+p0+ RF(" erased\n<a href='/'>back<a>"));
			} else server.send(403, RF("text/html"),RF("File erase protected and password didn't match !\n"));
		} else server.send(403, RF("text/html"),RF("No configuration available to server !\n"));
		return b;
	}


	/////////////////////////////////////
	bool servePermissionDenied(bool passwordprovided){
		if(!passwordprovided) server.send(403, RF("text/html"),RF("File access forbidden without password !\n"));
		else server.send(403, RF("text/html"),RF("File access forbidden, password incorrect !\n"));
		return true;
	} // data & log can be rendered uselessly read forbidden


	/////////////////////////////////////
	bool serveData(){
		// GenString message=myiodata->getAsJson();
		// case of sync events
		StringEvent strev=StringEvent();
		emit(RF(GET_JSON_DATA_EVENT),&strev);				// get the data by sync event rather than through getAsJson
		//			server.send(200, "application/json",strev.str.c_str(),strev.str.size());
		server.send(200, RF("application/json"),strev.str);	//is the server duplicating the string (no reason)
		return true;
	}




	/////////////////////////////////////
	bool serveDataNode(GenString &path){
		std::map<std::string,std::string> args=server.getArguments();
		if(args.empty()) {
			server.send(200, RF("text/html"),RF("NOTOK"));	// we send an http ok but a model notok
			return true;
		}

		GenString datapath=path.substr(5);	// minus "/data"
		StringMapEvent emap=StringMapEvent();
		for(auto it : args) {
			println(GenString()+"Found argument : '"+it.first+"'='"+it.second+"'");
			emap.values.set(datapath+'/'+it.first,it.second);
		}	// TODO: we should check if this argument is valid but how to know ? syntax-based ?

		println(GenString()+RF("IOServerv01::serveDataNode ")+emap.values.asJson());
		emit(RF("updateModel"),&emap);	//path is name of event object while reading is the event listened to

		if(!emap.ok) {
			server.send(200, RF("text/html"),RF("NOTOK"));  	// we send an http ok but a model notok
			return true;
		}

		server.send(200, RF("text/html"),RF("OK"));	//is the server duplicating the string (no reason)
		return true;
	}


	/////////////////////////////////////
	bool streamLogFile(GenString &path,unsigned int start,unsigned int end){
		GenString contentType=makeContentType(path);
		bool b=(server.streamFile(path, contentType,start,end)>0);
		if(b) println(GenString()+RF("Streamed : ")+path+RF(" ")+ contentType +RF(" : 1"));
		else println(GenString()+RF("Streamed : ")+path+RF(" ")+ contentType +RF(" : 0"));
		return b;
	}


	/////////////////////////////////////
	bool serveSetWifi(){
		GenString ssid=getArgument(RF("ssid"));
		GenString pwd=getArgument(RF("ssidpassword"));
		GenString mdnsname=getArgument(RF("mdnsname"));
		if(!ssid.empty() && !pwd.empty())
		{
			GenString str=GenString(RF("wifi-ssid: "))+ssid+RF("\nwifi-password: ")+pwd+"\n";
			if(!mdnsname.empty()) str+=RF("server-mdns-name: ")+mdnsname+"\n";
			CURFS.rewriteFile("/wificonfig.txt",(unsigned char *)str.c_str(), str.size());
			// or we reboot
			serverticker.once_ms(2000, rebootEsp);
			server.send(200, RF("text/html"),RF("Wifi ssid/password & mdns name successfully saved to /wificonfig.txt, rebooting..."));

			// or we try to load config and connect wifi without reboot (how does the server survives)
			// but we need a handle for that, configuration object is suppose to handle reconfiguration
			// each reconfigurable has a listener on the configuration object
			// when a configuration is changed, a tag is provided and used to the proper listener

			// there is also events

			return true;
		} else {
			server.send(200, RF("text/html"),RF("Please, provide wifi ssid/password to save to /wificonfig.txt !"));
			return true;
		}
	}


	/////////////////////////////////////
	bool servePopulate(){
		// we should check permission to populate
		if(serverpopulate || bootstrapDownloading)
			server.send (200, RF("text/html"),RF("Populating in progress - <a href='/'>back<a><br>")+
					to_string(bootstapFilesDownloaded)+"/"+to_string(bootstapFilesToDownload)+" files downloaded<br>"+
					bootstrapMessage);	// we show result in index page //until next populate or reboot
		else {
			filelisturl=getArgument(RF("filelisturl"));// save the arguments here
			filelistcontent=getArgument(RF("content"));
			serverpopulate=true;
			server.send (200, RF("text/html"),RF("Populating started now ... <br> <a href='/'>back<a>"));
		}
		return true;
	}





	bool serveRescuePage(){
		println("Serving rescue page");

		//// serve a backup page
		StringEvent strev=StringEvent();
		emit(RF("getStationIP"),&strev);	// get the data by sync event rather than through getAsJson

		// compile list of files into a html string
		GenString flist;
		std::vector<FileFS>fslist=CURFS.listFiles("/","");
		if(fslist.empty()) flist=RF("<b>No files on Board !\n</b>");
		bool indexfound=false;
		for(FileFS fs:fslist){
			GenString fname=fs.name;
			if(fs.name==RF("/index.html") || fs.name==RF("/index.html.gz")) indexfound=true;
			//	println(GenString()+"Server found file :"+fs.name);
			flist+=RF("<li><a href='")+fs.name+RF("'>")+fs.name+RF("</a><font style='font-size:75%'> - ")+to_string(fs.size)+RF("bytes - </font><a href='/fileerase?path=")+fs.name+RF("' style='font-size:60%;' onclick='return confirm(\"Are you sure you want to erase this file?\")'>(erase)</a></li>");
		}
		if(!fslist.empty()) flist+=RF("</ul>");
		if(!indexfound) flist+=RF("<br>No index.html (or index.html.gz) detected on board ! <br>");

		// send title
		server.setContentLength(CONTENT_LENGTH_UNKNOWN);
		server.send (200, RF("text/html"), RF("<h2>IOktopus rescue page </h2>"));

		//send wifi status
		server.sendContent(RF("<h3>Wifi </h3>"));
		GenString ssid=config->get(RF("wifi-ssid"));
		// send also link to local ip, if connected
		if(!strev.str.empty()){
			server.sendContent(RF(SERVERBACKUPPAGEPRE1));
			server.sendContent(strev.str);
			server.sendContent(RF(SERVERBACKUPPAGEPRE2));
			server.sendContent(strev.str);
			server.sendContent(RF(SERVERBACKUPPAGEPRE3));
			if(!ssid.empty()) server.sendContent(GenString()+RF("Connected to wifi network ssid '")+ssid+"'<br>");
		} else {
			if(!ssid.empty()) server.sendContent(GenString()+RF("Could not connect to wifi network ssid '")+ssid+"'<br>");
			else server.sendContent(GenString()+RF("No wifi network ssid name provided<br>"));
		}

		server.sendContent(RF(SERVERBACKUPPAGE_WIFI));

		// send files
		server.sendContent(RF("<h3>Files on board </h3>"));
		server.sendContent(flist);

		// send populating progress
		//	if(bootstrapDownloading) server.sendContent("Populating in progress is slowing down the server, wait before requesting more pages !");
		if(!bootstrapMessage.empty()) {
			if(bootstrapDownloading) server.sendContent(RF("<h4>Populating in progress<h4>"));
			else server.sendContent(RF("<h4>Last populating<h4>"));
			//	server.sendContent(RF("<h4>Populating in progress<h4>"));
			server.sendContent(	to_string(bootstapFilesDownloaded)+"/"+to_string(bootstapFilesToDownload)+
					RF(" files downloaded<br><pre>")+bootstrapMessage+RF("</pre>"));
		}
		// send populating form
		server.sendContent(RF("<h4>Populate </h4>"));
		server.sendContent(RF(SERVERBACKUPPAGE_POPULATE));

		return true;
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

		emit(RF(GET_JSON_LOG_EVENT),&strev);

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



	void start(){	//https://tttapa.github.io/ESP8266/Chap10%20-%20Simple%20Web%20Server.html
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
		GenString mdnsname;
		if(config) mdnsname=config->get(RF(SERVERMDNSNAME_KEYWORD));
		if(mdnsname.empty()) mdnsname=RF(SERVERDEFAULTMDNSNAME);

		server.setMdnsName(mdnsname);

		server.begin();
		println(RF("IOktopus server started"));



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
