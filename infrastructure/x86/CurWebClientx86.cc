//#include "../../datastruct/GenString.h"
#include <iostream>
#include <Socket.h>

#include "CurWebClientx86.h"

//CurWebClientx86::CurWebClientx86(){};
/*
void CurWebClientx86::loadAsyncAll(std::string url,void(*cb)(CurWebClientx86*)){
	//this works synchronously is pseudo async
	open(url);
	std::string str=readString();
	while (str.size()>0){
		std::cout <<std::string("received content:'")+str+"'"<<std::endl;
		content+=str;
		str=readString();
	}
	close();
	if(cb) (*cb)(this);
};

void CurWebClientx86::open(std::string url){// e.g. http://192.168.2.5:8080/filelist.txt

	size_t found = url.find_first_of(":");
	std:: string protocol=url.substr(0,found);
	std::string url_new=url.substr(found+3); //url_new is the url excluding the http part
	size_t found1 =url_new.find_first_of(":");
	size_t found2 = url_new.find_first_of("/");
	std::string path =url_new.substr(found2);
	std::string host,port;
	if(found1>url_new.size()) {
		host=url_new.substr(0,found2);port="80";
	}
	else {
		host=url_new.substr(0,found1);
		port =url_new.substr(found1+1,found2-found1-1);
	}
	if(protocol=="https") {
		port="443";
		socketClient=new SocketClient(host,std::stoi(port));

		std::string sendbuf = std::string("GET ")+path+"  /HTTP/1.1\r\nHost: "+host+"\r\nConnection: close\r\n\r\n";

		socketClient->SendLine(sendbuf);

		std::string str=socketClient->ReceiveLine();
		std::string str2=socketClient->ReceiveLine();
		std::string str3=socketClient->ReceiveLine();
		while(str.length()>0 && str!="\r\n"){
			std::cout << std::string("received header:'")+str+"'"<<std::endl;
			headers+=str;
			str=socketClient->ReceiveLine();
		}
	} else {
		socketClient=new SocketClient(host,std::stoi(port));

		std::string sendbuf = "GET "+url+"  /HTTP/1.1\r\nHost: "+host+":"+port+"\r\nConnection: close\r\n\r\n";

		socketClient->SendLine(sendbuf);

		std::string str=socketClient->ReceiveLine();

		while(str.length()>0 && str!="\r\n"){
			std::cout << std::string("received header:'")+str+"'"<<std::endl;
			headers+=str;
			str=socketClient->ReceiveLine();
		}
	}




};



std::string CurWebClientx86::readString(){

	return socketClient->ReceiveBytes();
};

unsigned int CurWebClientx86::read(unsigned char *ptr, unsigned int size){
	if(buffer.empty()) buffer=socketClient->ReceiveBytes();
	unsigned int s=size;
	if(buffer.size()<s) s=buffer.size();
	for(unsigned int i=0;i<s;i++) *(ptr+i)=buffer[i];
	buffer=buffer.substr(s);
	return s;
};

void CurWebClientx86::close(){
	socketClient->Close();

};

*/
