 #include <FS.H>
 #include <ESP8266HTTPClient.h>
#include<vector>
 
 void saveFile(String url);
 
void saveFile(String url, String path);
 
void testDownload(){
	unsigned int step=0;
	std::vector<String> urlvect;
	//	urlvect.push_back("http://192.168.2.4:8080/data/filelist.txt","/filelist.txt");
//		urlvect.push_back("http://192.168.2.4:8080/data/config.txt","/config2.txt");
//		urlvect.push_back("http://192.168.2.4:8080/data/favicon.png","/favicon.png");

	saveFile("http://192.168.2.4:8080/data/filelist.txt","/filelist.txt");
	delay(10);
	saveFile("http://192.168.2.4:8080/data/config.txt","/config2.txt");
	delay(10);
 	saveFile("http://192.168.2.4:8080/data/favicon.png","/favicon.png");
/*	saveFile(urlvect[0]);
	saveFile(urlvect[1]);
	saveFile(urlvect[2]);*/
	
}

void saveFile(String url){
  unsigned int pathpos=url.indexOf("/",8);
  String path=url.substring(pathpos);
  saveFile(url,path);
  }

void saveFile(String url, String path){


    WiFiClient client;
    HTTPClient http;

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
      }
   //   http.end();



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
       
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
    delay(10);
  }


/*



void getAndPrintFile(String url){
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
	
	*/
