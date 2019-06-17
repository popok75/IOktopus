#ifndef CURWEBCLIENTESP8266
#define CURWEBCLIENTESP8266

#define WEBCLIENT_DEBUG 0
#define DEFAULT_BUFFER_SIZE 1024

#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>


#define CLIENTTIMEOUT 20000

class CWCYieldCallback{
public:
	virtual void yield(){};
};

class CWCCallback{	// not so many solutions :http://tedfelix.com/software/c++-callbacks.html
public:
	bool usefull=false;
	CWCYieldCallback* yieldcb=0;
	CWCCallback(CWCYieldCallback* yieldcb0=0):yieldcb(yieldcb0){};
	virtual bool pullFunc(uint8_t *buff, unsigned int length){println(GenString()+"CWCCallback::pullfunc "+to_string(length));return false;};	// return false to stop reading
	virtual void fullFunc(HTTPClient *http){};
	virtual void end(unsigned int leftbytes){};
	virtual void yield(){if(yieldcb) yieldcb->yield();};
};


class CurWebClientEsp8266 {

	std::string content;
	bool error=false;

public:
	std::string* getContent(){return &content;}

	bool printURL(std::string url, CWCYieldCallback* yieldcb=0){
		class printer : public CWCCallback {
		public:
			printer(CWCYieldCallback* yieldcb0=0) : CWCCallback(yieldcb0){};
			bool pullFunc(uint8_t *buff, unsigned int length){if(buff) Serial.write( buff,length);return true;};
		} myprinter(yieldcb);

		if(startsWith(url,"https:")) return loadHttpsURLTo(url,&myprinter);
		else if(startsWith(url,"http:")) return loadHttpURLTo(url,&myprinter);
	}

	bool loadURL(std::string url, CWCYieldCallback* yieldcb=0){
		class loader:public CWCCallback {
			CurWebClientEsp8266 *webclient;
		public:
			loader(CurWebClientEsp8266 *wc, CWCYieldCallback* yieldcb0): CWCCallback(yieldcb), webclient(wc) {};
			bool pullFunc(uint8_t *buff, unsigned int length){
				if(buff && webclient) webclient->content+=std::string((const char *)buff,length);
				return true;
			};
		} myloader(this,yieldcb);

		content="";
		if(startsWith(url,"https:")) return loadHttpsURLTo(url,&myloader);
		else if(startsWith(url,"http:")) return loadHttpURLTo(url,&myloader);
		return false;
	}
	/*
	struct SaveObj{
		bool saveinit=false;
		const char *path=0;
		File f;
	} *saveobj=0;
	 */


	bool saveURL(std::string url, std::string path, CWCYieldCallback* yieldcb=0){ // for http, we could use the full function the usual http.stream

		class saver:public CWCCallback {
			std::string *ppath;
			bool saveinit=false;
			File file;
		public:
			saver(CWCYieldCallback* yieldcb0, std::string *ppath0) : CWCCallback(yieldcb0), ppath(ppath0){};
			bool pullFunc(uint8_t *buff, unsigned int length){
				println(GenString()+"saveobj pullfunc "+to_string(length));
				if(buff){
					if(!saveinit){
					//	println("opening file in write mode");
						file = SPIFFS.open(ppath->c_str(), "w");
						if (file)  Serial.println(String()+RF("Could not open file for write ")+ppath->c_str());
						else Serial.println(String()+RF("Opened file for write ")+ppath->c_str());
						saveinit=true;
					}
					unsigned int bytesWritten=0;
				//	println(GenString()+"before write "+to_string(length));
					int bytesWrite=file.write(buff, length);
					bytesWritten += bytesWrite;
					println(GenString()+"written in file :"+to_string(bytesWrite));
					unsigned int bytesWriteu=0;
					if(bytesWrite>0) bytesWriteu=bytesWrite;
					// are all Bytes a writen to stream ?
					if(bytesWriteu != length) if(!retryWrite(file, buff, bytesWrite,length)){return false;}// is this necessary with spiffs ?
					// check for write error
					if(file.getWriteError()) {
						Serial.printf("Home-brew[HTTP-Client][writeToStreamDataBlock] stream write error %d\n", file.getWriteError());// is this necessary with spiffs ?
						return false;}
				}
				return true;
			};
			void end(unsigned int left){
				file.close();
				saveinit=false;
				if(left){
					Serial.println(String()+RF("Could not fully download '")+ppath->c_str()+"', deleting partial file");
					SPIFFS.remove(ppath->c_str());
				}
			};
		} mysaver(yieldcb,&path);


		bool b=false;
		if(startsWith(url,"https:")) b=loadHttpsURLTo(url,&mysaver);
		else if(startsWith(url,"http:")) b=loadHttpURLTo(url,&mysaver);
		return b;
	}


private:
	static bool retryWrite(File &f, uint8_t *buff, int bytesWrite, int bytesRead){
		{
			Serial.printf("Home-brew[HTTP-Client][writeToStream] short write asked for %d but got %d retry...\n", bytesRead, bytesWrite);

			// check for write error
			if(f.getWriteError()) {
				Serial.printf("Home-brew[HTTP-Client][writeToStreamDataBlock] stream write error %d\n", f.getWriteError());
				f.clearWriteError();//reset write error for retry
			}

			delay(1);// some time for the stream

			int leftBytes = (bytesRead - bytesWrite);
			bytesWrite = f.write((buff + bytesWrite), leftBytes);// retry to send the missed bytes
			if(bytesWrite != leftBytes) {
				// failed again
				Serial.printf("Home-brew[HTTP-Client][writeToStream] short write asked for %d but got %d failed.\n", leftBytes, bytesWrite);

				return false;
			}
			return true;
		}
	}


	//	bool loadHttpURLTo(std::string url, bool print=false, std::string *content=0, std::string *path=0){	// load url to this->content
	bool loadHttpURLTo(std::string url,CWCCallback *cbobj,
			unsigned int buffsize=DEFAULT_BUFFER_SIZE){

		WiFiClient client;
		HTTPClient http;

#if WEBCLIENT_DEBUG >1
		Serial.println("CurWebClientEsp8266::loadURLTo\n");
		Serial.print("[HTTP] begin...\n");
#endif
		String urlstr=url.c_str();
		error=true;

		if (!http.begin(client, urlstr)) {Serial.printf(RF("[HTTP} Unable to connect\n"));return !error;}

#if WEBCLIENT_DEBUG >1
		Serial.print("[HTTP] GET...\n");
#endif
		// start connection and send HTTP header
		int httpCode = http.GET();

		// httpCode will be negative on error
		if (httpCode <= 0) {Serial.printf(RF("[HTTP] GET... failed, error: %s\n"), http.errorToString(httpCode).c_str());return !error;}
		// HTTP header has been send and Server response header has been handled
#if WEBCLIENT_DEBUG >1
		Serial.printf("[HTTP] GET... code: %d\n", httpCode);
#endif
		int bytesWritten=0;
		// file found at server
		if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
			error=false;
			uint32_t lastPacket=0;
			if(cbobj && cbobj->usefull){
				cbobj->fullFunc(&http);
			} else if(cbobj && !cbobj->usefull){
				int buff_size = buffsize;
				WiFiClient *_client=http.getStreamPtr();
				int len = http.getSize();
				if((len > 0) && (len <buff_size )) buff_size = len;// if possible create smaller buffer then HTTP_TCP_BUFFER_SIZE

				uint8_t * buff = (uint8_t *) malloc(buff_size);
				if(buff){// read all data from server
					while(http.connected()  && (len > 0 || len == -1)) {
						size_t sizeAvailable = _client->available();// get available data size
						if(sizeAvailable) {
							int readBytes = sizeAvailable;
							if(len > 0 && readBytes > len) readBytes = len;// read only the asked bytes
							if(readBytes > buff_size) readBytes = buff_size;// not read more the buffer can handle
							//Serial.println("before readbytes ");Serial.println(readBytes);
							int bytesRead = _client->readBytes(buff, readBytes);// read data
#if WEBCLIENT_DEBUG >2
							Serial.println(String()+"[HTTP] readbytes "+String(bytesRead));
#endif
							//	Serial.write(buff, bytesRead);

							if(cbobj) cbobj->pullFunc(buff,bytesRead);

							if(len > 0) len -= bytesRead;
							bytesWritten+=bytesRead;
							lastPacket=0;
							delay(0);
						} else {
							delay(1);
							if(!lastPacket) lastPacket=millis();
						}
				/*		Serial.print("len:");
						Serial.println((uint32_t)len);
						Serial.print("sizeAvailable:");
						Serial.println((uint32_t)sizeAvailable);
						Serial.print("cbobj:");
						Serial.println((uint32_t)cbobj);
				*/		if(cbobj) cbobj->yield();

						if(lastPacket && len>0 && (millis()-lastPacket)>CLIENTTIMEOUT) break;// timeout here: if receive nothing 20sec, consider the connection dead
					}

					if(cbobj) cbobj->end(len);
					if(len) {
						error=true;// if finish before len=0 should at least display an error
						Serial.println(String()+RF("[HTTP] connection stopped before the end, ")+String(len)+RF(" bytes left to download\n"));
					}

					free(buff);
				} else Serial.println(RF("[HTTP] no memory to allocate the buffer for downloading.\n"));
			}
		} else Serial.printf(RF("[HTTP] GET... failed, error: %s\n"), http.errorToString(httpCode).c_str());



		http.end();
#if WEBCLIENT_DEBUG >1
		Serial.println(String()+"\n[HTTP] closed after reading "+String(bytesWritten)+" bytes");
#endif

		return !error;
	}




/* quintessence
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
void setup(){
	WiFiClient client;
			HTTPClient http;
http.begin(client, "");
int httpCode = http.GET();
WiFiClient *_client=http.getStreamPtr();
http.connected();
_client->available();
uint8_t buff[128];
_client->readBytes(buff, 0);
http.end();

WiFiClientSecure sclient;
sclient.setInsecure();
sclient.connect("", 80);
sclient.print("");
sclient.connected();
sclient.readStringUntil('\n');
sclient.read(buff,sizeof(buff));
 */















	bool loadHttpsURLTo(std::string url, CWCCallback *cbobj, unsigned int buffsize=DEFAULT_BUFFER_SIZE){ //,bool print=false,std::string *content=0, std::string *spiffspath=0){
		const int httpsport=443;
		size_t found = url.find_first_of(":");
		std::string protocol=url.substr(0,found);
		std::string url_new=url.substr(found+3);
		size_t found1 =url_new.find_first_of(":");
		if(found1>url_new.size()) found1 =url_new.find_first_of("/");
		size_t found2 = url_new.find_first_of("/");
		String host =url_new.substr(0,found1).c_str();
		String path =url_new.substr(found2).c_str();

		WiFiClientSecure client;
		// WiFiClient client;

		Serial.print("CurWebClientEsp8266::loadSecureURLTo:[HTTPS] connecting to ");
		Serial.println(host);

		client.setInsecure();// client.setFingerprint(fingerprint);// Serial.printf("Using fingerprint '%s'\n", fingerprint);
		if (!client.connect(host, httpsport)) {
			Serial.println("CurWebClientEsp8266::loadSecureURLTo:[HTTPS] connection failed");
			error=true;
			return !error;
		}

		//String url = "/repos/esp8266/Arduino/commits/master/status";
		//  String url = "http://192.168.2.5:8080/filelist.txt";
		Serial.print("CurWebClientEsp8266::loadSecureURLTo:[HTTPS] requesting URL: ");
		Serial.println(url.c_str());

		client.print(String("GET ") + path + " HTTP/1.1\r\n" +
				"Host: " + host + "\r\n" +	//			"User-Agent: BuildFailureDetectorESP8266\r\n" +
				"Connection: close\r\n\r\n");

		while (client.connected()) {
			String line = client.readStringUntil('\n');
			if (line == "\r") {

#if WEBCLIENT_DEBUG >1
				Serial.println("CurWebClientEsp8266::loadSecureURLTo:[HTTPS] headers received");
#endif

				break;
			}
		}
		uint8_t buff[buffsize];// = { 0 };

		while(client.connected()){//i>0){
			int i=client.read(buff,sizeof(buff));
			if(i>0){
#if WEBCLIENT_DEBUG >1
				Serial.println(String("CurWebClientEsp8266::loadSecureURLTo:[HTTPS] received:"));
#endif
				if(cbobj) if(!cbobj->usefull) cbobj->pullFunc(buff,i);
				//Serial.write(buff,i);
				//	justWrite(buff,i);
				delay(0);
				if(cbobj) cbobj->yield();
			} else {

				delay(1);
				if(cbobj) cbobj->yield();
			}
		}
		if(cbobj) if(!cbobj->usefull) cbobj->pullFunc(0,0);	//maybe we should make a special function for end too ?
		return true;

	}




};












/*

	//adpated from https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266HTTPClient/src/ESP8266HTTPClient.cpp
	int writeToStreamDataBlock(HTTPClient* http)
	{
		int buff_size = 128;
		int bytesWritten=0;
		WiFiClient *_client=http->getStreamPtr();
		int len = http->getSize();

		if((len > 0) && (len <buff_size )) buff_size = len;// if possible create smaller buffer then HTTP_TCP_BUFFER_SIZE

		uint8_t * buff = (uint8_t *) malloc(buff_size);
		if(buff){
			// read all data from server
			while(http->connected()  && (len > 0 || len == -1)) {
				size_t sizeAvailable = _client->available();// get available data size
				if(sizeAvailable) {
					int readBytes = sizeAvailable;
					if(len > 0 && readBytes > len) readBytes = len;// read only the asked bytes
					if(readBytes > buff_size) readBytes = buff_size;// not read more the buffer can handle
					Serial.println("before readbytes ");
					Serial.println(readBytes);
					// read data
					int bytesRead = _client->readBytes(buff, readBytes);
					Serial.println("after readbytes");

					Serial.write(buff, bytesRead);
					if(len > 0) len -= bytesRead;
					bytesWritten+=bytesRead;
					delay(0);
				} else delay(1);

			}
			free(buff);
			Serial.println("writeToStreamDataBlock connection closed.\n");
		} else Serial.println("writeToStreamDataBlock no memory to allocate the buffer for downloading.\n");
		return bytesWritten;
	}


	/ *				Serial.println("before get string");
							String payload = http.getString();
							Serial.println("after get string");
							Serial.println(payload);
 */
/*
							uint8_t buff[buffsize];// = { 0 };
							WiFiClient* stream = http.getStreamPtr();
							int len = http.getSize();
							// read all data from server
							while (http.connected() && (len > 0 || len == -1)) {
								// get available data size
								Serial.println("before stream->available");
								size_t size = stream->available();
								if (size) {
									// read up to buffer size
									Serial.println("before stream->readBytes");
									int c = stream->readBytes(buff, buffsize);
									Serial.println("after stream->readBytes");
									Serial.println();
									Serial.write(buff, c);
									if (len > 0) len -= c;

								}
								delay(1);
								Serial.print("-");
							}
							Serial.println();*/
/*
							if(client.connected()) Serial.println("client connected ");
							else Serial.println("client not connected ");
							while(client.connected()){//i>0){
								Serial.println("client reading ");
								int i=client.read(buff,sizeof(buff));
								if(i>0){
	#if WEBCLIENT_DEBUG >1
									Serial.println(String()+"[HTTP] received:"+String(i));
									Serial.write(buff,i);
									Serial.println();
	#endif

									if(pullfunc) (*pullfunc)(buff,i,obj);
								} else if(pullfunc) (*pullfunc)(0,0,obj);
								// delay(1);
							}
}*/
/*					if(path){
							File f = SPIFFS.open(path->c_str(), "w");
							if (f) {
								http.writeToStream(&f);
							} else  Serial.println(String()+RF("Could not open file for write ")+path->c_str());
							f.close();
							// read back
							/ *						File dataFile = SPIFFS.open(path.c_str(), "r");   //Ouverture fichier pour le lire
							Serial.println(String()+"Lecture du fichier '"+path.c_str()+"' en cours:");
							if(dataFile){//Affichage des donnÃ©es du fichier
								for(int i=0;i<dataFile.size();i++)Serial.print((char)dataFile.read());    //Read file
							} else Serial.println("Could not open file for read");
							dataFile.close();
 */
/*} else {
							String payload = http.getString();	//works only for small files, if we display a big we should use a buffer
							if(content) *content+=payload.c_str();
							if(print) Serial.println(payload);
						}*/



















/*

	void loadURL(std::string url){//,void(*cb)(CurWebClientEsp8266*)=0) {
			Serial.println("loadURL::"+url);
		//	Serial.println("timing start");
			unsigned int mil=millis();
			open(url);
	//		Serial.println("timing 1:"+String(millis()-mil));mil=millis();

			std::string str=readString();
	//		Serial.println("timing 2:"+String(millis()-mil));mil=millis();

			while (str.size()>0){
	#if DEBUGCLIENT>0
	//			Serial.print(String("[HTTP] received content:")+String(str.c_str()));
	#endif
				//std::cout <<std::string("received content:'")+str+"'"<<std::endl;
				content+=str;
				str=readString();
	//			Serial.println("timing n:"+String(millis()-mil));mil=millis();

			//	delay(1);
			}
	//		Serial.println("timing 3:"+String(millis()-mil));mil=millis();

			close();
	//		Serial.println("timing 4:"+String(millis()-mil));mil=millis();

			//if(cb) (*cb)(this);



		};


	void showDownloadFile(String url){
	     HTTPClient http;

	    WiFiClient client;

	    Serial.print("[HTTP] begin...\n");


	    // configure server and url
	    http.begin(client, url);
	    //http.begin(client, "jigsaw.w3.org", 80, "/HTTP/connection.html");

	    Serial.print("[HTTP] GET...\n");
	    // start connection and send HTTP header
	    int httpCode = http.GET();
	    if (httpCode > 0) {
	      // HTTP header has been send and Server response header has been handled
	      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

	    if (httpCode == HTTP_CODE_OK) {
	      showFile(http,client);
	      }
	    } else {
	      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
	    }

	    http.end();

	  }
	 void showFile(HTTPClient &http,WiFiClient &client){
	     // get lenght of document (is -1 when Server sends no Content-Length header)
	        int len = http.getSize();

	        // create buffer for read
	        uint8_t buff[128] = { 0 };

	        // get tcp stream
	       WiFiClient * stream = &client;

	      // read all data from server
	        while (http.connected() && (len > 0 || len == -1)) {
	          // get available data size
	          size_t size = stream->available();

	          if (size) {
	            // read up to 128 byte
	            int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

	            // write it to Serial
	            Serial.write(buff, c);

	            if (len > 0) {
	              len -= c;
	            }
	          }
	     //     delay(1);
	        }

	        Serial.println();
	        Serial.print("[HTTP] connection closed or file end.\n");
	    }

	void saveFile(String url, String path){
	     HTTPClient http;

	    WiFiClient client;

	    Serial.print("[HTTP] begin...\n");


	    // configure server and url
	    http.begin(client, url);
	    //http.begin(client, "jigsaw.w3.org", 80, "/HTTP/connection.html");

	    Serial.print("[HTTP] GET...\n");
	    // start connection and send HTTP header
	    int httpCode = http.GET();

	    if (httpCode > 0) {
	      // HTTP header has been send and Server response header has been handled
	      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

	      // file found at server
	      if (httpCode == HTTP_CODE_OK) {
	         File f = SPIFFS.open(path, "w");
	         if (f) {
	                http.writeToStream(&f);
	         } else  Serial.println("Could not open file for write");
	         f.close();

	         File dataFile = SPIFFS.open(path, "r");   //Ouverture fichier pour le lire
	         Serial.println(String()+"Lecture du fichier '"+path+"' en cours:");
	         if(dataFile){
	        	 //Affichage des données du fichier
	        	 for(int i=0;i<dataFile.size();i++)
	        	 {
	        		 Serial.print((char)dataFile.read());    //Read file
	        	 }
	         } else Serial.println("Could not open file for read");
	         dataFile.close();
	      }

	    } else {
	      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
	    }

	   http.end();

	  }


	void saveUrlToFile(std::string url,std::string path, void(*cb)(CurWebClientEsp8266*)=0) {
		//saveFile(url.c_str(),path.c_str());
		showDownloadFile(url.c_str());
		/ *
		open(url);
		if(!error) streamToFile(path);
		close();* /
		if(cb) cb(this);
	}

	void streamToFile(std::string &path){
		if(http){
			 File f = SPIFFS.open(path.c_str(), "w");
			 if (f) {
					http->writeToStream(&f);
			 } else  Serial.println("Could not open file for write");
			 f.close();
		}
	}




	//	CurWebClientEsp8266();
	void open(std::string url){
		size_t found = url.find_first_of(":");
		std::string protocol=url.substr(0,found);

		if(protocol=="http"){
			client=new WiFiClient();
			http=new HTTPClient();

		//	http->setReuse(true);
			String urlstr(url.c_str());
#if DEBUGCLIENT>0
			Serial.print(String()+"[HTTP] begin..."+urlstr);
#endif
//			Serial.println("open timing start");
			unsigned int mil=millis();

			http->begin(*client, urlstr);// configure server and url

//			Serial.println("open timing 1: "+String(millis()-mil));mil=millis();
			//http.begin(client, "jigsaw.w3.org", 80, "/HTTP/connection.html");
#if DEBUGCLIENT>0
			Serial.print("[HTTP] GET...\n");
#endif
			// start connection and send HTTP header
			httpCode = http->GET();

//			Serial.println("open timing 2: "+String(millis()-mil));mil=millis();
			if (httpCode > 0) {
				// HTTP header has been send and Server response header has been handled
#if DEBUGCLIENT>0
				Serial.printf("[HTTP] GET... code: %d\n", httpCode);
#endif
				// file found at server
#if DEBUGCLIENT>0
				if (httpCode == HTTP_CODE_OK) {
					error=false;
					Serial.println("[HTTP] code ok");
				}
#endif
			} else {error=true;
#if DEBUGCLIENT>0
			Serial.print("[HTTP] code : ");
			Serial.println(httpCode);
			Serial.printf("[HTTP] GET... failed, error: %s\n", http->errorToString(httpCode).c_str());
#endif
			}
		} else if(protocol=="https") {
			const int httpsport=443;
			std::string url_new=url.substr(found+3);
			size_t found1 =url_new.find_first_of(":");
			if(found1>url_new.size()) found1 =url_new.find_first_of("/");
			std::string host =url_new.substr(0,found1);
			size_t found2 = url_new.find_first_of("/");
			std::string path =url_new.substr(found2);
#if DEBUGCLIENT>0
			Serial.print("[HTTPS] begin...\n");
#endif

			clientsecure=new WiFiClientSecure();
			clientsecure->setInsecure();
			if (!clientsecure->connect(host.c_str(), httpsport)) {
				Serial.println(String("connection failed to host:")+host.c_str()+" on port "+String(httpsport));
				return;
			}
#if DEBUGCLIENT>0
			Serial.print("[HTTPS] requesting URL: "); Serial.println(path.c_str());
#endif
			clientsecure->print(String("GET ") + path.c_str() + " HTTP/1.1\r\n" +
					"Host: " + host.c_str() + "\r\n" +
	//				"User-Agent: BuildFailureDetectorESP8266\r\n" +
					"Connection: close\r\n\r\n");
#if DEBUGCLIENT>0
			Serial.println("[HTTPS] request sent");
#endif
			//String content;
			while (clientsecure->connected()) {
				String line = clientsecure->readStringUntil('\n');
				if (line == "\r") {
#if DEBUGCLIENT>0
					Serial.println("[HTTPS] headers received "+line);
#endif
					break;
				}
				headers+=line.c_str();
			}


			/ *
			  String line = client.readStringUntil('\n');
			  if (line.startsWith("{\"state\":\"success\"")) {
			    Serial.println("esp8266/Arduino CI successfull!");
			  } else {
			    Serial.println("esp8266/Arduino CI has failed");
			  }
			  Serial.println("reply was:");
			  Serial.println("==========");
			  Serial.println(line);
			  Serial.println("==========");
			  Serial.println("closing connection");* /
		} else return;//no suitable protocol






	};



	unsigned int read(unsigned char *ptr, unsigned int size);
	std::string readString(){
		if(client){
			// get lenght of document (is -1 when Server sends no Content-Length header)
			int len = http->getSize();

			// create buffer for read
			uint8_t buff[128] = { 0 };
			// get tcp stream
			WiFiClient * stream = client;
			std::string str;
			// read all data from server
			if (http->connected() && (len > 0 || len == -1)) {
				// get available data size
				size_t size = stream->available();
				if (size) {
					// read up to 128 byte
					int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
#if DEBUGCLIENT>0
					Serial.println(String("read size:")+String(c)+", content:");
					// write it to Serial
					Serial.write(buff, c);
#endif
					str+=std::string((const char *)buff);
					if (len > 0) {
						len -= c;
					}
				}
				//delay(1);
			}
			return str;
		}
		if(clientsecure){
			unsigned int buffsize=128;
			uint8_t buff[buffsize] ;
			std::string line;
			if(clientsecure->connected()){//i>0){
				int i=clientsecure->read(buff,buffsize);
				if(i>0){
					std::string strl((const char*)buff,i);
					Serial.println(String("received:")+strl.c_str());
					line += strl.c_str();
				}
			}
			return line;
		}
	};

	void close(){
		if(http) {
			http->end();
#if DEBUGCLIENT>0
		Serial.print("[HTTP] connection closed or file end.\n");
#endif
		}
		//if(clientsecure) clientsecure->close();
	};
 */

















/*
bool saveURLTo(std::string url,std::string path){

	WiFiClient client;
	HTTPClient http;
#if WEBCLIENT_DEBUG >0
	Serial.println(String()+"CurWebClientEsp8266::saveURLTo "+path.c_str()+" from "+url.c_str());
	Serial.print("[HTTP] begin...\n");
#endif
	// configure server and url
	String urlstr=url.c_str();

	if (http.begin(client, urlstr)) {
#if WEBCLIENT_DEBUG >0
		Serial.print("[HTTP] GET...\n");
#endif
		// start connection and send HTTP header
		int httpCode = http.GET();

		if (httpCode > 0) {
			// HTTP header has been send and Server response header has been handled
#if WEBCLIENT_DEBUG >0
			Serial.printf("[HTTP] GET... code: %d\n", httpCode);
#endif
			// file found at server
			if (httpCode == HTTP_CODE_OK) {
				File f = SPIFFS.open(path.c_str(), "w");
				if (f) {
					http.writeToStream(&f);
					error=false;
				} else  Serial.println(String()+RF("Could not open file for write ")+path.c_str());
				f.close();
			}

		} else {
			Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
			error=true;
		}
		http.end();
#if WEBCLIENT_DEBUG >0
		Serial.printf("[HTTP} closed\n");
#endif
		/ *	if (httpCode == HTTP_CODE_OK) {
			File dataFile = SPIFFS.open(path.c_str(), "r");   //Ouverture fichier pour le lire
			Serial.println(String()+"Lecture du fichier '"+path.c_str()+"' en cours:");
			if(dataFile){
				//Affichage des donnÃ©es du fichier
				for(int i=0;i<dataFile.size();i++)
				{
					Serial.print((char)dataFile.read());    //Read file
				}
			} else Serial.println("Could not open file for read");
			dataFile.close();
		}* /

	}else {
		Serial.printf("[HTTP} Unable to connect\n");
	}

	return !error;
}

 */

#endif


