#ifndef ACTIONPROCESS_H
#define ACTIONPROCESS_H

#include "OperationLib.h"

/*
 * FullActionProcess complement Action process with useful functions;

 * BasicActionProcess extends FullActionProcess by
 * 		- separating the events into init, enable, disable,
 * 		- adding protection against self call
 */
#undef FTEMPLATE
#define FTEMPLATE ".irom.text.fullactionprocess"

#define ARGSNAME "args"
#define RESULTSNAME "results"

class FullActionProcess : public ActionProcess {
public:
	// available to user-defined action process
	std::vector<GenString> getArgumentNames(GenString argname); // get the list of variable names from the definition file
	GenString getSubArgumentName(GenString argname, unsigned int index);
	GenString getArgumentValue(GenString argname);
	virtual void updateArgumentValue(GenString argname, GenString val);
	GenString getOperationPath();
	IODatav03 *getModel();
	bool isOperationActive();


};


GenString FullActionProcess::getArgumentValue(GenString argname){
	GenString path=parentAction->getOperation()->getPath()+'/'+argname;
	return parentAction->getOperation()->getOpLib()->getModel()->get(path);
};



std::vector<GenString> FullActionProcess::getArgumentNames(GenString argname){
	// this maybe should flatten the arrays into src/0 dst/1 and not return only src or dst
	// maybe it should be done inside OperationDefs
	localDefAction ldefop=localDefAction(parentAction);
	GenString p=parentAction->getClassName()+'/'+argname;
	std::vector<GenString> arglist=
			parentAction->getOperation()->getOpLib()->getDefinitions()->getArgumentList(p,&ldefop);
	return arglist;
};

GenString FullActionProcess::getSubArgumentName(GenString argname, unsigned int index){
	//derive a name for subargument
	return getPathBranch(argname)+'/'+to_string(index);
};

bool FullActionProcess::isOperationActive(){
	return parentAction->getOperation()->isActive();
};

GenString FullActionProcess::getOperationPath(){return parentAction->getOperation()->getPath();};

IODatav03* FullActionProcess::getModel(){return parentAction->getOperation()->getOpLib()->getModel();};

void FullActionProcess::updateArgumentValue(GenString argname, GenString val){
	parentAction->getOperation()->getOpLib()->getModel()->updateVal(getOperationPath()+'/'+argname,val);
};



class BasicActionProcess : public FullActionProcess {
private:
	//bool enabledOperation=true;	// to call enable/disable, must not be modified by derived classes

	bool selfcalling=false;
	ticket_t myticket=0;

public:
	bool inited=false;
	bool react=true;

	bool isEnabled(){return react;}
	virtual void init(){};
	virtual void enable(){};	// called when operation active change state, restore events
	virtual void disable(){};	//called when operation active change state, also stop events, can be overloaded
	virtual void basicprocess(std::vector<GenString> argpaths,std::vector<GenString> resultpaths, GenString eventpath, Event *e=0)=0;	// each notification goes here

	void updateArgumentValue(GenString argname, GenString val){
		// implement tickets
		selfcalling=true;
		FullActionProcess::updateArgumentValue(argname,val);
		myticket=getModel()->getEmitTicket();
		selfcalling=false;	//this is useless in async context
	};

	virtual void process(GenString path, Event *e=0){

		if(selfcalling) return; // this prevent the action from notifying itself, a self operation activating/disactivating action would need to override selfcalling
		if(myticket>0 && getModel()->getPropagateTicket()<=myticket) {
			return;	// prevent self notification asynchronous
		}
//		std::cout << "current model "<< getModel()->getAsJson()<<std::endl;
		bool active=isOperationActive();
		if(!inited) {react=active;inited=true;init();}	// at init, we initialize and we react straight away
		if(react  && !active) {react=false;disable();}
		if(!react  && active) {react=true;enable();}
		if(react) {	// if react is overriden in disable, allow to receive events when not active
			std::vector<GenString> args=getArgumentNames(RF(ARGSNAME));
			std::vector<GenString> results=getArgumentNames(RF(RESULTSNAME));
	//		for(GenString name:args) std::cout << "BasicActionProcess:: arg: " << name <<", val: "<<getArgumentValue(name) <<std::endl;
	//		for(GenString name:results) std::cout << "BasicActionProcess:: result: " << name <<", val: "<<getArgumentValue(name) <<std::endl;
			basicprocess(args,results,path,e);
		}
	};	// each notification goes here

	// to implement by  user-defined action process
	//	virtual ActionProcess* duplicate()=0;
	//	virtual bool named(GenString name)=0;
};


#endif








