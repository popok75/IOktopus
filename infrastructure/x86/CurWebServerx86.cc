
#include <iostream>
#include <fstream>


//#include "monoTimer.h"

#include<future>

//extern
unsigned int threadNumber;

std::mutex mut;



class TaskStruct;

// extern  template <typename... T> TaskStruct*  monoSetTimeOut(unsigned int ms, bool repeat, void(fc)(T...),T... params);

extern TaskStruct*  monoSetTimeOut(unsigned int ms, bool repeat, void(fc)());
//extern TaskStruct*  monoSetTimeOut(unsigned int ms, bool repeat, void(fc)(int),int);
//extern TaskStruct*  monoSetTimeOut(unsigned int ms, bool repeat, void(fc)(int,std::string),int,std::string);

extern void yield();

std::mutex newthreadmut;

template <typename... ParamTypes>
void anotherThread(void (func)(ParamTypes...), ParamTypes... parames)
{
	mut.lock();
	threadNumber++;
	mut.unlock();
	std::cout<<"anotherThread:1 Starting a new thread"<< std::endl;
	std::thread([func,parames...]()
			{
		std::cout<<"anotherThread:3 Started a new thread"<< std::endl;
//		newthreadmut.unlock();
		func(parames...);
		mut.lock();
		threadNumber--;
		mut.unlock();
			}).detach();
/*	newthreadmut.lock();
	newthreadmut.lock();
	newthreadmut.unlock();
*/	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	std::cout<<"anotherThread:2 Created a new thread"<< std::endl;
}	//freeze if we dont wait : the function should not return before the thread start or it kills the lambda


// pull out the type of messages sent by our config
//typedef server::message_ptr message_ptr;




////////////////////
#include "./webserver/webserver.h"
#include "Socket.h"
#include "CurWebServerx86.h"


#define HTTP_GET 1

struct httpconnHandler{
	webserver::http_request* req;
	httpconnHandler(webserver::http_request *req0):req(req0){};
};
struct httpserverHandler{
	webserver *ppserver;
	httpserverHandler(webserver *ppserver0):ppserver(ppserver0){};
};


CurWebServerx86 *serverinst=0;

CurWebServerx86::CurWebServerx86(unsigned int port0):port(port0){
	port=port0;
	serverinst=this;
};

void  CurWebServerx86::handleClient(){yield();}

std::string CurWebServerx86::header(std::string name){      // get request header value by name
	std::string str=name+": ";
	size_t pos=lastreq->req->allheaders.find(str);
	if(pos!=std::string::npos){
		size_t pos2=lastreq->req->allheaders.find("\n",pos+str.size());
		if(pos2==std::string::npos) pos2=lastreq->req->allheaders.size();
		return lastreq->req->allheaders.substr(pos+str.size(),pos2-pos-str.size());
	}
	return std::string("");
};

bool CurWebServerx86::hasHeader(std::string name){       // check if header exists
	std::string str=name+":";
	size_t pos=lastreq->req->allheaders.find(str);
	bool f= (pos!=std::string::npos);
	return f;
};
void socketsend(webserver::http_request *req, const char *buffer,std::size_t size,int ){
	send(req->s_->s_,buffer,size,0);
}
void  CurWebServerx86::sendHeader( std::string header, std::string value){
	if(!lastreq->req->xtraheader.empty()) lastreq->req->xtraheader+="\r\n";
	lastreq->req->xtraheader+=header+": "+value ;
}

void CurWebServerx86::onHttp(std::string uri){
	lasturi=uri;

	// must remove right part of ?
	if(callbacks.find(lasturi)!=callbacks.end()) (callbacks[lasturi])();
	else if(callbacks.find("notfound")!=callbacks.end()) (callbacks["notfound"])();
};

std::map<std::string,std::string> CurWebServerx86::getArguments(){
	std::map<std::string,std::string> stm;
	for(auto &iter:lastreq->req->params_){
		stm[iter.first]=iter.second;
	}
	return stm;
}
void  CurWebServerx86::sendContent(std::string message){
	lastreq->req->answer_ += message;
}
void  CurWebServerx86::send(int status, std::string type, std::string message){
	std::string body;
	lastreq->req->contentType=type;
	if(status==404) {
		lastreq->req->status_ = "404 Not Found";
		body       = message; //lastreq->status_;
	}
	if(status==200){
		lastreq->req->status_ = "200 OK";
		body       = message;
	}
	//		lastreq->answer_  = "<html><head><title>";
	//		lastreq->answer_ += lastreq->status_;
	//		lastreq->answer_ += "</title></head><body>";
	lastreq->req->answer_ += body;
	//		lastreq->answer_ += "</body></html>";
	//		if(status==404) lastcon->set_status(websocketpp::http::status_code::not_found,message);
	/*
	 */
};
/*
void  CurWebServerx86::keepopen(){

}
*/
std::string cppreplaceAll(std::string src, std::string pattern,std::string newval){
	std::string::size_type n = 0;
	while ( ( n = src.find( pattern, n ) ) != std::string::npos )
	{
		src.replace( n, pattern.size(), newval );
		n += pattern.size();
	}
	return src;
}

bool endsWith2(std::string const &fullString, std::string const &ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
	} else
		return false;
};


size_t CurWebServerx86::streamFile(std::string filename, std::string contentType, unsigned long start,unsigned long stop){
	std::string diskpath =  programpath, path=filename;
	path=cppreplaceAll(path,"/","\\");
	if(path.substr(0,1)=="\\") path=path.substr(1,path.size());
	diskpath+=path;
	if (contentType == ".gz" ||
			(endsWith2(filename, ".gz") && contentType != "none" && contentType !="application/octet-stream"))
	     {sendHeader("Content-Encoding", "gzip");}

	std::ifstream myfile(diskpath, std::ios::in|std::ios::binary|std::ios::ate);
	std::size_t size = 0;

	if (myfile.is_open())
	{
		char* oData = 0;
		size = myfile.tellg();
		myfile.seekg(start, std::ios::beg);
		unsigned int buffsize=size-start;
		if(stop>0 && stop<size) buffsize=stop-start;
		oData = new char[buffsize];
		myfile.read(oData, buffsize);
		//	std::cout << "red:"<< red <<std::endl;;
		//oData[size] = '\0';
		std::cout << "CurWebServerx86::streamFile Loaded to send bytes "<< buffsize << " from file " << diskpath << "["<<start<<"-"<<(start+buffsize)<<"]"<<std::endl;
		std::string mess="HTTP/1.1 200 OK\r\nContent-Length: "+std::to_string(buffsize);
		if(!contentType.empty()) mess+="\r\nContent-Type: "+contentType;
		if(!lastreq->req->xtraheader.empty()) mess+="\r\n"+lastreq->req->xtraheader;

		mess+="\r\n\r\n";
		socketsend(lastreq->req,mess.c_str(),(std::size_t)mess.size(),0);

		socketsend(lastreq->req,oData,buffsize,0);
		myfile.close();
		// should close socket and tell websocket to not continue writing
		lastreq->req->status_="toclose";
		delete oData;
	}
	return size;
};




bool serverbusy=false;
std::mutex servermutex;

void callServer(){
	printf("2.5 webserver callback\n");
	serverinst->onHttp(serverinst->lasturi);
	printf("2.5 webserver post callback\n");
	servermutex.lock();
	serverbusy=false;
	servermutex.unlock();
}

/*********************************************************************************/
void Request_Handler(webserver::http_request* r) {

	while(serverbusy)
		std::this_thread::sleep_for(std::chrono::milliseconds(10)); // serialize response

	if(!serverbusy) {
		servermutex.lock();
		serverbusy=true;
		servermutex.unlock();

		serverinst->lasturi=r->path_;
		serverinst->lastreq=new httpconnHandler(r);
//		printf("Request_Handler:: 1.webserver asking timeout\n");
		monoSetTimeOut(1,false,callServer);
//		printf("Request_Handler:: 2.webserver func going to wait\n");
		while(serverbusy)
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
//		printf("Request_Handler:: 3.webserver waited and going to exit\n");

	}
}


/**********************************************************************/





void callServerReceiver(CurWebServerx86*ser){
	ser->run();
}

void CurWebServerx86::begin(){
	if(!started) started=true;
	else return;

	anotherThread(callServerReceiver,this);

}

void CurWebServerx86::run(){

	ppserver=new httpserverHandler(new webserver(port, Request_Handler));
};








