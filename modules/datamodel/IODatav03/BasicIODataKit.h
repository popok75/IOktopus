#ifndef BASICIODATAKIT_H
#define BASICIODATAKIT_H

#include "FullActionProcess.h"


#include "../../../infrastructure/CompatFS.h"

#define NODESPATH RF("/nodes/")

/*	This file is the most user relevant part of IODatav03 model: BasicActionProcess class can be extended by user programmed actions/processes.
 *
 * BasicIODataKit
 * 		This file helps to setup a IODatav03 data model. BasicIODataKit will create the IODatav03 model, then create and add OperationLib as a plug-in.

 *	BasicActionProcess implementations:
 *		This file also contain basic custom ActionProcess below. These processes are notified according to the arguments declared in definitions.
 *		Here is the hardcoded process part that use the argument to make the process and produce a result saved in the data model
 * */


////////////////////

/*
 * CompareProcess will update 'conditionmet' according to 'operator' and 'lefthand' and 'righthand'
 */

#define COMPAREPROCESSNAME "compare"

class CompareProcess : public BasicActionProcess {
public:
	ActionProcess* duplicate(){return new CompareProcess();};
	void init(){inited=true;}
	virtual void basicprocess(std::vector<GenString> args,std::vector<GenString> results,GenString path, Event *e){
		std::cout << "process compare :"<< path << ", my path :" <<getOperationPath()<<std::endl;

		if(args.size()==3 && results.size()){// check we have the right number of argument
			GenString left=getArgumentValue(args[0]);
			GenString right=getArgumentValue(args[1]);
			GenString operat=getArgumentValue(args[2]);
			bool operless=false;
			if(!isDigit(left) || !isDigit(right)) return;	// error arguments are not number/ comparable, should we extend to alphabetic in this case ?
			if(startsWith(operat,RF("less"))) operless=true;
			else if(startsWith(operat,RF("greaterorequal"))) operless=false;
			else return;	// operator not recognized
			double dleft=strToDouble(left);
			double dright=strToDouble(right);
			char result;
			if(operless && dleft<dright) result='1';
			if(!operless && dleft>=dright) result='1';
			else result='0';
			updateArgumentValue(results[0],GenString()+result);//delay response to avoid congesting event processing ?
			for(GenString name:results) std::cout << "result:" << name <<", val:"<<getArgumentValue(name) <<std::endl;
		}

	};	// each notification goes here
	virtual bool named(GenString name){if(name==RF(COMPAREPROCESSNAME)) return true; else return false;};
};

/*
 * SelectAssign will copy the content of src[channel] to dst[channel] whenever src[channel] or channel changes
 */

#define SELECTASSIGNPROCESSNAME "selectassign"
class SelectassignProcess  : public BasicActionProcess {

public:
	ActionProcess* duplicate(){return new SelectassignProcess();};

	virtual void basicprocess(std::vector<GenString> args,std::vector<GenString> results,GenString path, Event *e){
		std::cout << "process selectassign : "<< path << ", my path : " <<getOperationPath()<<std::endl;
		// maybe we should test if link or get raw value for both args and results
		if(results.empty()) return;									// error : result var not defined
		GenString channel=getArgumentValue(args[0]);
		if(!isDigit(channel)) return;									// error : channel not number
		unsigned nchannel=strToUnsignedLong(channel);
		if(nchannel<=(args.size()-2)){
			GenString valtocopy=getArgumentValue(args[nchannel+1]);
			GenString sub=getSubArgumentName(results[0],nchannel);
			updateArgumentValue(sub,valtocopy);
			std::cout << "-result: " << sub <<", val: "<<getArgumentValue(sub) <<std::endl;
			for(GenString name:results) std::cout << "result: " << name <<", val: "<<getArgumentValue(name) <<std::endl;
		}
	};	// each notification goes here
	virtual bool named(GenString name){if(name==RF(SELECTASSIGNPROCESSNAME)) return true; else return false;};
};

#include "ClocktriggerProcess.h"


class MinMaxModule: public IODataListener
{
public:
	IODatav03 *datamodel=0;

	virtual bool notify(GenString path,GenString  tag, Event*event=0) {
		if(!datamodel) return false;
		if(getPathLeaf(path)!=RF(VALUEFIELD)) return false;
		GenString val=datamodel->get(path);
		if(!isDigit(val)) return false;

		bool b=false;
		GenString omin=datamodel->get(path+SLASH+MINFIELD);
		GenString omax=datamodel->get(path+SLASH+MAXFIELD);
		GenString min=getMin(omin,val);//			println("Omin:"+omin);
		GenString max=getMax(omax,val);//			println("Omax:"+omax);

		if(min!=omin) {datamodel->updateVal(path+SLASH+MINFIELD,min);b=true;}	// should trigger a notification at some point
		if(max!=omax) {datamodel->updateVal(path+SLASH+MAXFIELD,max);b=true;}
		return b;
	};

};


class BasicIODataKit{
public:
	static void exportInto(OperationLib *oplib){
		ActionProcess *ap=new CompareProcess();
		oplib->addActionProcess(ap);
		ap=new SelectassignProcess();
		oplib->addActionProcess(ap);
		ap=new ClocktriggerProcess();
		oplib->addActionProcess(ap);
	}

	static  IODatav03* createIODatav03AndCo(){
		GenString actionfile="datafiles/action_defs.json";
		GenString opfile="datafiles/operation_defs.json";
		GenString opdatafile="datafiles/operation_data.json";
		return setupModel(opdatafile, opfile, actionfile);
	};

	static IODatav03* setupModel(GenString opdatafile,GenString opfile,GenString actionfile){
		IODatav03* datamodel=new IODatav03();

		MinMaxModule *minmaxmod=new MinMaxModule();
		datamodel->on(NODESPATH,WRITETAG,minmaxmod);

		OperationLib *oplib=new OperationLib();
		unsigned int s;

		datamodel->addPlugIn(oplib);
		datamodel->loadJSONContent(CURFS.readFileToString(opdatafile,s));

		std::cout << "DataModel :" << std::endl;
		std::cout << datamodel->getAsJson() << std::endl;


		oplib->getDefinitions()->loadOperationJSONContent(CURFS.readFileToString(opfile,s));
		oplib->getDefinitions()->loadActionJSONContent(CURFS.readFileToString(actionfile,s));

		exportInto(oplib);

		oplib->build();	// subscribe to nodes/op values and conform op data

		std::cout << "DataModel :" << std::endl;
		std::cout << datamodel->getAsJson() << std::endl;

		return datamodel;
	}
};



#endif









