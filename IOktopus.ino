
/// IOktopus main launcher
/*
	For testing sensors use :
		- DHT22 :
*/
#ifndef x86BUILD
#define ESP8266BUILD// need to redefine FTEMPLATE every few headers // https://www.bountysource.com/issues/46510369-using-pstr-and-progmem-string-with-template-class-causes-section-type-conflict //#define FF(string_literal) (reinterpret_cast<const __FlashStringHelper *>(((__extension__({static const char __c[] __attribute__((section(".irom.text.template"))) = ((string_literal)); &__c[0];})))))//#define RF(x) String(FF(x)).c_str()
#define FFX(string_literal) (reinterpret_cast<const __FlashStringHelper *>(((__extension__({static const char __c[] __attribute__((section(FTEMPLATE))) = ((string_literal)); &__c[0];})))))
#define RF(x) String(FFX(x)).c_str()
#else
#define RF(x) x
#endif




#include "datastruct/GenString.h"

#include "datastruct/GenMap.H"
#include "datastruct/GenTreeMap.H"

#include "infrastructure/CompatPrint.h"
#include "infrastructure/events/DefaultEventEmitter.H"
#include "infrastructure/SyncedClock.h"


//#include "infrastructure/CompatNet.h"
#include "infrastructure/Configuration.h"
#include "infrastructure/CompatCrash.h"
#include "infrastructure/ErrorLogFile.h"

#include "modules/datamodel/IODataFactory.h"
#include "modules/server/IOServerFactory.h"
#include "modules/logger/IOLoggerFactory.h"
#include "modules/device/IODeviceFactory.h"


#include <algorithm>



#define WIFIPASSFILE "/wificonfig.txt"


IOServerGen *ioserver=0;
IODataGen *iodata=0;
IODeviceGen *iosensor=0;
IOLoggerGen *iologger=0;


Configuration config;
WifiMan wifiman;

void loadDevices();


#undef FTEMPLATE
#define FTEMPLATE ".irom.text.main"

GenString getTimestampString(){
	return CLOCK32.getNowAsString()+RF("-UTC");
};

Ticker mainticker;

const char* ssid = "Gardening, cheaper than therapy"; //replace this with your WiFi network name
const char* password = "seeds freedom"; //replace this with your WiFi network password

void setup(){

	initPrint();

	//	testDSB();
	//	return;
	CurSaveCrash.setTimestampFunc(&getTimestampString);
	if(CURFS.exists(CRASHFILEPATH)){
		println(RF("\n#################### Crash dump file start"));
	//	unsigned int fs=0;
	//	CURFS.printFile(CRASHFILEPATH,fs);// temporarily disabled, useful only in deployment
		println(RF("#################### Crash dump file end"));

	} else println(RF("\nNo crash dump saved"));



	//	to_string(0);
	//	to_string(0xFFFFFFFFFFFFFFFF);


	/*
	GenString crashstr=CurSaveCrash.print();
	if(!crashstr.empty()){
		println(RF("Crash dump print : \n")+crashstr);
	} else println(RF("No crash saved"));
	 */
	println();
	//CurSaveCrash.clear();
	/*	delay(10000);
	Serial.println("Attempting to divide by zero ...");
	    int result, zero;
	    zero = 0;
	    result = 1 / zero;
	    Serial.print("Result = ");
	    Serial.println(result);
	 */
	// list all files
	println("File list start");
	CURFS.printFiles();
	println("File list end");
	config.loadTxtFile(RF(WIFIPASSFILE));
	config.loadTxtFile(RF("/config.txt")); // +256sram -824 freeheap

	// connect to ssid/make an ap
	wifiman.initFromConfig(&config.configmap);
	/*
	// Connect to WiFi network
	  WiFi.mode(WIFI_STA);
	  WiFi.begin(ssid, password);
	  Serial.println("");

	  // Wait for connection
	  while (WiFi.status() != WL_CONNECTED) {
	    delay(500);
	    Serial.print(".");
	  }
	  Serial.println("");
	  Serial.print("Connected to ");
	  Serial.println(ssid);
	  Serial.print("IP address: ");
	  Serial.println(WiFi.localIP());
	/ *
	if(!CURFS.exists(CRASHFILEPATH)){
		//crash
		 Serial.println("Attempting to divide by zero ...");
		        int result, zero;
		        zero = 0;
		        result = 1 / zero;
		        Serial.print("Result = ");
		        Serial.println(result);
		println(RF("\nNo crash saved"));
	}
	 */
	//CURFS.eraseAllFiles();


//	return;

	//delay(5000);

	// create data model
	iodata=IODataFactory::create();	//create v0.1

	// this init data is now created by the device from config file
	// create a map with init data	// add it to model
	//	GenMap map={{"type","input"}, {"unit","\%"}, {"val","--"}};GenString path="/nodes/Humidity";
	//	iodata->update(path, map);	GenString jsonstr=iodata->getAsJson();

	// create server// develop client done
	ioserver= IOServerFactory::create("IOktopus-Server", "v0.1",&config.configmap);

	//	ioserver->setIOData(iodata); // classic way to do the stuff, bad because creates dependency between server and data
	ioserver->on("getAsJson",iodata); // instead we set iodata to be called on getAsJson event fired by the server (we use sync event to get the result back immediately)
	ioserver->on("getStationIP",&wifiman);	// server will fire getStationIP toward wifiman in order to include in backup page
//	println("ioktopus starting ioserver");
	ioserver->start();

	// create log
	iologger= IOLoggerFactory::create(&config.configmap);
	iodata->on(RF("modelUpdated"),iologger);	// modelUpdated event fired by data will trigger iologger
	ioserver->on(RF("getLogJson"),iologger); 	// the server will use events to get the log as json, again using sync event result

	// load devices
	loadDevices();

	errLog("IOktopus started successfully");
	println("\n\nIOktopus started successfully !\n");



	//	downloadFile();
//	mainticker.once_ms(1000, testDownload);



	//testDownload();
	//return;
	/*
 	config.configmap.set("device0-model","DHT22");
	config.configmap.set("device0-pins","D5");
	config.configmap.set("device0-autoreader","5");
	IODeviceFactory::create(0,&config.configmap, RF("updateModel"),iodata);
	 */
	//	config.configmap.set("device1-model","Psychrometer");
	//	config.configmap.set("device1-autoreader","5");
	//	IODeviceFactory::create(1,&config.configmap, RF("updateModel"),iodata);
	/*
 	config.configmap.set("device2-model","DS18B20");
	config.configmap.set("device2-pins","D7");
	config.configmap.set("device2-autoreader","5");
	IODeviceFactory::create(2,&config.configmap, RF("updateModel"),iodata);

	config.configmap.set("device2-model","Psychrometer");
	config.configmap.set("device2-pins","D7, D7");
	config.configmap.set("device2-autoreader","5");
	IODeviceFactory::create(2,&config.configmap, RF("updateModel"),iodata);
	 *//*
	config.configmap.set("device3-model","HTU21");
	config.configmap.set("device3-pins","D2, D1");
	config.configmap.set("device3-autoreader","5");
	IODeviceFactory::create(3,&config.configmap, RF("updateModel"),iodata);
	  *//*
	config.configmap.set("device4-model","SHT15");
	config.configmap.set("device4-pins","D0, D3");
	config.configmap.set("device4-autoreader","5");
	IODeviceFactory::create(4,&config.configmap, RF("updateModel"),iodata);

	// create sensor
	/ *
		IODeviceGen*iosensor4= IODeviceFactory::create(0,"Outdoor","DHT22",{D5},
					{{"temperature","DHT22-temperature"},{"humidity","DHT22-Humidity"}},"/nodes/","v0.1");
		iosensor4->on("updateModel",iodata);
		iosensor4->init();
		iosensor4->getReader()->autoread(5);
	   */	/*	IODeviceGen*iosensor0= IODeviceFactory::create(0,"Outdoor","HTU21",{D2,D1},
				{{"temperature","HTU21-temperature"},{"humidity","HTU21-Humidity"}},"/nodes/","v0.1");
		iosensor0->on("updateModel",iodata);
		iosensor0->init();
		iosensor0->getReader()->autoread(5);

	/ *	iosensor= IODeviceFactory::create(0,"Outdoor","SHT15",{D0,D3},
				{{"temperature","SHT15-temperature"},{"humidity","SHT15-Humidity"}},"/nodes/","v0.1");
		iosensor->on("updateModel",iodata);
		iosensor->init();
		iosensor->getReader()->autoread(5);

/ *		IODeviceGen* iosensor2= IODeviceFactory::create(1,"Fridge","Psychro-DSB18B20",{D7,D7},{{"humidity","BulbHumidity"},{"temperature-wet","WetBulb"},{"temperature-dry","DryBulb"}},"/nodes/","v0.1");
		iosensor2->on("updateModel",iodata);
		iosensor2->init();
		iosensor2->getReader()->autoread(5);
		/ *
		IODeviceGen* iosensor2= IODeviceFactory::create(1,"Fridge","DSB18B20",{D7},{{"path","WetBulb"}},"/nodes/","v0.1");
		iosensor2->on("updateModel",iodata);
		iosensor2->init();
		iosensor2->getReader()->autoread(5);

		IODeviceGen* iosensor3= IODeviceFactory::create(2,"Fridge","DSB18B20",{D7},{{"path","DryBulb"}},"/nodes/","v0.1");
		iosensor3->on("updateModel",iodata);
		iosensor3->init();
		iosensor3->getReader()->autoread(5);
	    */
#ifdef ESP8266
	print(RF("<--------main::setup - memory at end of setup:"));	 println(ESP.getFreeHeap(),DEC);
#endif


}
void loop(){
	// Serial.print(".");
	//   Serial.print("loop : free memory :");  Serial.println(ESP.getFreeHeap(),DEC);
	if(ioserver) ioserver->yield();
	yield();
	delay(10);	//slow down server reaction compared to the rest ?
}
/*
unsigned int getPin(GenString pinstr){
	unsigned int p=255;
	while(pinstr[0]==' ') pinstr.erase(0,1);
	while(pinstr[pinstr.length()-1]==' ') pinstr.erase(pinstr.length()-1,1);
	if(pinstr==RF("D0")) p=D0;
	if(pinstr==RF("D1")) p=D1;
	if(pinstr==RF("D2")) p=D2;
	if(pinstr==RF("D3")) p=D3;
	if(pinstr==RF("D4")) p=D4;
	if(pinstr==RF("D5")) p=D5;
	if(pinstr==RF("D6")) p=D6;
	if(pinstr==RF("D7")) p=D7;
	return p;
}

std::vector<unsigned int>getPins(GenString pins){
	std::vector<unsigned int> vect;
	unsigned int ip=0,i=pins.find(",",ip);
	while(i<pins.size()){
		GenString sub=pins.substr(ip,i);
		unsigned int p=getPin(sub);
		if(p!=255) {
			vect.push_back(p);
			println(GenString()+RF("getPins(): added pin:")+sub+RF(" ")+to_string(p));
		}
		ip=i+1;
		i=pins.find(",",ip);
	}
	if(ip<pins.size()){
		GenString sub=pins.substr(ip);
		unsigned int p=getPin(sub);
		if(p!=255) {
			vect.push_back(p);
			println(GenString()+RF("getPins(): added pin:")+sub+RF(" ")+to_string(p));
		}
	}

	return vect;
}
 */
long getDeviceNum(GenString key){
	if(startsWith(key,RF("device"))){
		unsigned int i=key.find("-",6);
		if(i<key.size()){
			GenString sub=key.substr(6,i-6);
			if(isDigit(sub)){
				long l=strToUnsignedLong(sub);
				return l;
			}
		}

	}
	return -1;
}

void loadDevices(){
	// for each device-n in the config map make a new iodevice
	std::vector<int> donevect;
	for(auto it:config.configmap){
		GenString key=it.key();
		int num=getDeviceNum(key);
		if(num>=0) {
			auto p=std::find(donevect.begin(),donevect.end(),num);
			if(p==donevect.end()){
				IODeviceFactory::create(num,&config.configmap, RF("updateModel"),iodata);
				donevect.push_back(num);
			}
		}
	}
}
/*
void oldloadDevices(){


	struct DeviceConf {
		unsigned int num;
		//GenString name;
		GenString model;
		GenString autoread;
		std::vector<unsigned int> pins;
		GenMap paths;//=GenSSMap();
	};
	std::vector<DeviceConf> devices;
	//	println(GenString()+"loadDevices : ");
	for(auto it: config.configmap){		// make a list per device
		//		println(GenString()+"loadDevices : iterate");
		GenString key=it.key();
		println(GenString()+"key : "+key);
		if(startsWith(key,RF("device"))){
			key.erase(0,6);
			unsigned int i=key.find('-');
			if(i<key.size()){
				GenString dnum=key.substr(0,i);
				unsigned int dnui=strToUnsignedLong(dnum);
				DeviceConf *cdev=0, ndev;
				for(DeviceConf &dev:devices) if(dev.num==dnui) cdev=&dev;
				if(!cdev) {devices.push_back(ndev);cdev=&devices[devices.size()-1];cdev->num=dnui;}
				key.erase(0,i+1);
				//if(key=="name") {cdev->name=it.value();}
				if(key==RF("autoread")) {cdev->autoread=it.value();}
				if(key==RF("model")) {cdev->model=it.value();}
				if(key==RF("pins")) {cdev->pins=getPins(it.value());}
				if(startsWith(key,RF("path"))) {
					if(key.size()>5) cdev->paths.set(key.substr(5),it.value());
					else cdev->paths.set(RF("path"),it.value());
				}
			}
		}
	}

	for(DeviceConf dc:devices){
		IODeviceGen *iosensor= IODeviceFactory::create(dc.num,RF("sensor")+to_string(dc.num),dc.model,dc.pins,dc.paths,RF("/nodes/"),RF("v0.1"));
		if(iosensor){
			iosensor->on(RF("updateModel"),iodata);
			iosensor->init();
			if(!dc.autoread.empty()) {
				unsigned int sec=strToUnsignedLong(dc.autoread);
				iosensor->getReader()->autoread(sec);
			}
		}
	}
}
 */




void end(){
	// clean up
	if(ioserver) delete ioserver;
	if(iodata) delete iodata;
}
