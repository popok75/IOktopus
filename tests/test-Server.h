
void connectWifi(){
	WiFi.begin(ssid, password);

	  Serial.println();
	  Serial.print("Connecting");
	  while (WiFi.status() != WL_CONNECTED)
	  {
	    delay(500);
	    Serial.print(".");
	  }

	  Serial.println("success!");
	  Serial.print("IP Address is: ");
	  Serial.println(WiFi.localIP());
}

bool testClock(){
	Serial.begin(115200);
	  delay(1000);
	  SPIFFS.begin();
	  Serial.println(); Serial.print("Configuring access point...");
	  setupWiFi();


	  server.on("/", HTTP_GET, []() {
	      handleFileRead("/");
	    });

	    server.onNotFound([]() {                          // Handle when user requests a file that does not exist
	      if (!handleFileRead(server.uri()))
	        server.send(404, "text/plain", "FileNotFound");
	    });

	    webSocket.begin();                                // start webSocket server
	    webSocket.onEvent(webSocketEvent);                // callback function

	    server.begin();
	    Serial.println("HTTP server started");
	    yield();
	return true;
};



