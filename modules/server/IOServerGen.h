#ifndef IOSERVERGEN_H
#define IOSERVERGEN_H

#include "../../infrastructure/CompatNet.h"
#include "../../infrastructure/CompatPrint.h"
#include "../datamodel/IODataGen.h"

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.ioservergen"

class IOServerGen : public DefaultEventEmitter
{

public:
	CurWebServer server;
	IODataGen *myiodata=0;

	GenString servername;

	IOServerGen (unsigned int httpport=80): server(httpport){servername=RF("IOktopus-Server");println(RF("Constructor IOServerGen"));};
	IOServerGen (GenString name, unsigned int httpport=80): server(httpport),servername(name){println(RF("Constructor IOServerGen"));};
	virtual ~IOServerGen(){};

	void setIOData(IODataGen *myiodata1){myiodata=myiodata1;}

	virtual bool handleRequest(std::string path){return false;}

	virtual void start(){};

	virtual void yield(){server.handleClient();};
};



#endif
