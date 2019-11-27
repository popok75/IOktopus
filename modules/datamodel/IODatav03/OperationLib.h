#ifndef OPERATIONLIB_H
#define OPERATIONLIB_H


#include "IODatav03.h"

#include "OperationDefs.h"

/* 	OperationLib
 	 -  an IODataPlugin : in iodataplugin list.
 	 - Contain current operations :
 	  	  - Operation is created from data and definitions, including it's own Actions
 	  	  - Action is created from data and definition. Action contain ActionProcess that encapsulates the end-user process/notification
 	  	  - OperationLib contains the definitions, and the actionImplementations
	 - during Action and Operation creation, data model variables are "conformed" to the definitions, i.e. created and initialized if not found
	 - the ActionProcess inputs/outputs are encoded in text file by default. This helps keep the hardcoded part minimal
	 - In "OperationDefs.h" is defined all definition files interpretation, variable conformance, action and operation argument names

 * */

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.operationlib"

////////////////////// Operation lib

#define MODELOPPATH "/operations/"

////////// ActionProcess

class Operation;
class Action;


class ActionProcess {
protected:
	Action *parentAction=0;
public:
	void setParentAction(Action *parent){parentAction=parent;}
	// to implement by  user-defined action process
	virtual ActionProcess* duplicate()=0;
	virtual void process(GenString path, Event *e=0)=0;	// each notification goes here
	virtual bool named(GenString name)=0;
};



//////////////////////////
class OperationLib: public IODataPlugin {
	OperationDefs opdefs;
	std::vector<Operation*> operations;	//should be destroyed at the end
	std::vector<ActionProcess*> actionprocessesprotos; //should be destroyed at the end
public:

	void addActionProcess(ActionProcess *ap){actionprocessesprotos.push_back(ap);}
	ActionProcess *createActionProcess(GenString type){
		for(ActionProcess *ap : actionprocessesprotos){if(ap->named(type)) return ap->duplicate();}
		return 0;
	}
	OperationDefs* getDefinitions(){return &opdefs;}
	Operation* getOperation(GenString opname);
	IODatav03 *getModel(){return datamodel;}


	bool build();

	Operation *createOperation(GenString &subpath);
	Action* createAction(GenString &classname, Operation *op);
};












/////////// Action class
class Action : public IODataListener{
	Operation *parentOperation=0;
	ActionProcess *aProcess;
	GenString className;
public:
	Action(GenString classname0, Operation *parent):parentOperation(parent),className(classname0){}
	virtual bool notify(GenString path, GenString  tag, Event*event=0){
	//	std::cout << "Action::notify " << path << std::endl;
		if(aProcess) aProcess->process(path,event);
	//	else std::cout << "Action::notify no aProcess" << path << std::endl;
		return true;
	};
	bool init();

	GenString getClassName(){return className;}

	Operation* getOperation(){return parentOperation;}

	bool build();


};




















#define CLASSKEYWORD "class"

/////////// Operation class
class Operation : public IODataListener {
	GenString name;
	OperationLib *oplib;
	std::vector<Action*> actions;
public:
	Operation(GenString name0,OperationLib *oplib0){name=name0;oplib=oplib0;}
	GenString getPath(){return GenString(RF(MODELOPPATH))+name;}
	GenString getName(){return name;}

	virtual bool notify(GenString path, GenString  tag, Event*event=0){
	//	std::cout << "Operation::notify " << path << std::endl;
		// distribute notify to inner actions
		for(Action *ac : actions) ac->notify(path,tag,event);
		return true;
	};

	OperationLib *getOpLib(){return oplib;}

	class localDefOperation : public DefOperation {
		Operation *myOperation;
	public:
		localDefOperation(Operation*op):myOperation(op){};
		virtual IODataListener *getListener(){return myOperation;};
		virtual IODatav03 *getModel(){return myOperation->oplib->getModel();};
		virtual GenString getModelFullPath(){return myOperation->getPath();};
	} ;

	bool isActive(){
		GenString vardefpath=GenString(RF(OPPATHDEFLIB))+"Operation/vars";
		OperationDefs *gdefs=oplib->getDefinitions();
		std::vector<GenString> keys = gdefs->getDefs(vardefpath);
		if(keys.empty()) return false;
			// first arg of generic operation must be active field name
		GenString active=oplib->getModel()->get(getPath()+'/'+keys.front());
		return active=="1";
	}

	bool build(){
	//	std::cout << "Operation::build " << name << std::endl;
		auto tm=oplib->getModel();
		localDefOperation ldefop=localDefOperation(this);
		// conform data with definitions
		//get definitions
		GenString opclass=tm->get(getPath()+"/"+RF(CLASSKEYWORD));
		OperationDefs *gdefs=oplib->getDefinitions();

		gdefs->conform("Operation", &ldefop);
		gdefs->conform(opclass, &ldefop);

		/*	//check result
		std::vector<GenString> subolist=tm->listSubpaths(RF(MODELOPPATH)+fullpath);

		for (GenString subsubpath:subolist) {
			GenString valpath=RF(MODELOPPATH)+fullpath+"/"+subsubpath;
			std::cout<< valpath<< " : "<<tm->get(valpath) << std::endl;
		}
		 */
		// must subscribe to relevant events : which? how ? "args" vars through IODataEmitter left to write, that keeps a genmap of "path" IODataListener
		gdefs->subscribe("Operation", &ldefop);
		gdefs->subscribe(opclass, &ldefop);

		// must create and build each contained action
		GenString vardefpath=GenString(RF(OPPATHDEFLIB))+opclass+"/actions";
		std::vector<GenString> keys = gdefs->getDefs(vardefpath);
		for(GenString k:keys){
			GenString subval =gdefs->getDefVal(vardefpath+"/"+k);
			std::vector<GenString> subkeys =gdefs->getDefs(vardefpath+"/"+k);
			//	std::cout<<   " action name "<<k << " subval " <<subval <<" "<<subkeys.size() << std::endl;
			Action *ac=oplib->createAction(subval, this);
			ac->build();
			actions.push_back(ac);
		}

		for(Action *ac:actions){ //init actions
			ac->init();
		}


		return true;
	}


};




//////////////// Action subfunctions
bool Action::init(){
//	std::cout << "Action::init " << parentOperation->getPath() << std::endl;
	if(aProcess) aProcess->process(parentOperation->getPath(),0);
//	else std::cout << "Action::init no aProcess" << parentOperation->getPath() << std::endl;
	return true;
}

//////////////////////
class localDefAction : public DefAction {
	Action *myAction;
public:
	localDefAction(Action*ac):myAction(ac){};
	virtual IODataListener *getListener(){return myAction;};
	virtual IODatav03 *getModel(){return myAction->getOperation()->getOpLib()->getModel();};
	virtual GenString getModelFullPath(){return myAction->getOperation()->getPath();};
};

bool Action::build(){
	// conform & subscribe
//	std::cout << "Action::build " << className << std::endl;
	localDefAction ldefop=localDefAction(this);
	OperationDefs *gdefs=parentOperation->getOpLib()->getDefinitions();
	gdefs->conform("action", &ldefop);
	gdefs->conform(className, &ldefop);

	/*	GenString fullpath=parentOperation->getPath();
		//check result
	auto tm=parentOperation->getOpLib()->getModel()->getDataTreeMap();
	std::vector<GenString> subolist=tm->listSubpaths(RF(MODELOPPATH)+fullpath);

	for (GenString subsubpath:subolist) {
		GenString valpath=RF(MODELOPPATH)+fullpath+"/"+subsubpath;
		std::cout<< valpath<< " : "<<tm->get(valpath) << std::endl;
	}
	 */
	// must subscribe to relevant events : which? how ? "args" vars through IODataEmitter left to write, that keeps a genmap of "path" IODataListener
	gdefs->subscribe("action", &ldefop);
	gdefs->subscribe(className, &ldefop);


	aProcess=parentOperation->getOpLib()->createActionProcess(className);
	if(aProcess!=0) {aProcess->setParentAction(this);}

	return true;
}




//////////////// OperationLib subfunctions
Operation *OperationLib::getOperation (GenString opname){
	for(Operation *op:operations){
		if(op->getName()==opname) return op;
	}
	return 0;
}

Operation *OperationLib::createOperation(GenString &subpath){
	Operation *op=new Operation(subpath, this);
	return op;
}


Action* OperationLib::createAction(GenString &classname, Operation *op){
	Action *ac=new Action(classname, op);
	return ac;
}

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.operationlib2"


bool OperationLib::build(){	// build the operation lib : create operation/actions, subscribe them and conform the data
	if(!datamodel) return false;

	// get list of all operations in datamodel

	std::vector<GenString> olist=datamodel->subPaths(RF(MODELOPPATH));
	for(GenString subpath:olist) {
		//std::cout<< "operation name " <<subpath<<std::endl;
		Operation *op=getOperation(subpath);
		if(!op){ //create
			op=createOperation(subpath);
			op->build();
		}
	}

	return true;
}


#endif








