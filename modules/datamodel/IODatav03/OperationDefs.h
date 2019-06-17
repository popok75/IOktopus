#ifndef OPERATIONDEFS_H
#define OPERATIONDEFS_H


#include "IODatav03.h"

/*	OperationDefs
 *  These classes are used by operation lib to interpret the definition files. These files are currently JSON files that are translated into a GenTreeMap.
 *  Operation definition file define generic Operation and specific Operations and their components :
 *  	"var" is subscribed to
 *  	"init" is just initialized
 *  	"actions" are the name of actions to create.
 *	Action definition file define generic Action and specific Actions and their arguments :
 *		"args" are the arguments actions subscribe to
 *		"results" are the variables produced by the action, are not substribed to
 *		"desc" user oriented text description of the functioning of the action
 *		N.B: "args" and "results" are required to use JSON arrays because arguments will be accessed by index and not by name from the action process
 * */

////////////////////// Operation lib

#define OPPATHDEFLIB "/operations/"
#define ACPATHDEFLIB "/actions/"

#define OPKEYWORD "operation"
#define ACTIONKEYWORD "action"

// defoperation & defaction classes could be an argument containing full path and listener, eventually model too//
// eventually how to build the conform and subscribe full path

class DefItem {
public:
	virtual IODataListener *getListener()=0;
	virtual IODatav03 *getModel()=0;
	virtual GenString getModelFullPath()=0;

	virtual std::vector<GenString> subscribePaths(GenString vardefpath)=0;
	virtual std::vector<GenString> conformPaths(GenString vardefpath)=0;
	virtual GenString getPathBase()=0;
};

class DefOperation : public DefItem {
public:
	std::vector<GenString> subscribePaths(GenString vardefpath){return {RF(OPPATHDEFLIB)+vardefpath+"/vars"};};
	std::vector<GenString> conformPaths(GenString vardefpath){
		return {getPathBase()+vardefpath+"/vars", getPathBase()+vardefpath+"/init"};
	}
	GenString getPathBase(){return RF(OPPATHDEFLIB);};
};

class DefAction : public DefItem{
public:
	std::vector<GenString> subscribePaths(GenString vardefpath){return {RF(ACPATHDEFLIB)+vardefpath+"/args"};};
	std::vector<GenString> conformPaths(GenString vardefpath){
		return {getPathBase()+vardefpath+"/args", getPathBase()+vardefpath+"/results"};
	}
	GenString getPathBase(){return RF(ACPATHDEFLIB);};
};



////////////////////////////////
class OperationDefs {
	GenTreeMap definitions;
public:
	std::vector<GenString> getDefs(GenString subpath){
		std::vector<GenString> vect;
		std::vector<GenString> subs=definitions.listSubpaths(subpath);	//return names not paths
		for(GenString it: subs){
			vect.push_back(getPathLeaf(it));
		}
		if(subs.empty()){
			GenString str=definitions.get(subpath);
			if(!str.empty()) vect.push_back(str);
		}
		return vect;
	};
	GenString getDefVal(GenString subpath){
		return definitions.get(RF(subpath));
	}

	bool loadOperationJSONContent(GenString jsoncontent){
		bool b= loadJSONToGenTreeMap(jsoncontent,&definitions);
		std::cout << "OperationDefs::loadOperationJSONContent :"<<definitions.getAsJson() << std::endl;
		return b;
	};

	bool loadActionJSONContent(GenString jsoncontent){
		bool b= loadJSONToGenTreeMap(jsoncontent,&definitions);
		std::cout << "OperationDefs:: actions loaded :" << definitions.getAsJson() << std::endl;
		return b;
	};


	bool conformValue(GenString varmodelpath, GenString &varval, DefItem *defitem){
		// check if model has already this var
		// if not create it with def value
		varmodelpath=defitem->getModelFullPath()+"/"+varmodelpath;
		IODatav03 *tm=defitem->getModel();

		// 	GenString mval=tm->get(varmodelpath);
		if(!tm->hasPath(varmodelpath)){//mval.empty()) {
			tm->updateVal(varmodelpath,varval);
	//		std::cout << "OperationDefs:: conformed : "<<varmodelpath<<", new val :"<< tm->get(varmodelpath) << std::endl;
		} else {
			GenString val=tm->get(varmodelpath);
	/*		if(!val.empty()) std::cout << "OperationDefs:: conformValue found existing path : "<<varmodelpath<<", with val :"<< val << std::endl;
			else {
				std::cout << "OperationDefs:: conformValue found existing path : "<<varmodelpath<< std::endl;
				GenString pro=tm->progeny(varmodelpath);
				while(!pro.empty()){
					std::cout << "--OperationDefs:: existing sub path : "<<pro<<", with val :"<< tm->get(pro) << std::endl;
					pro=tm->progeny(varmodelpath,pro);
				}
			}*/
		}

		//mval=tm->get(varmodelpath);
		return true;
	}

	bool testNumeral(GenString str){	// return true if only numbers
		for(char c:str){if(c<'0' || c>'9') return false;}
		return true;
	}

	std::vector<GenString> getArgumentList(GenString &argname,DefItem *defitem){
		// this will return the argument names and values
				// as found in definition
				// as instantiated in data
		GenString path=defitem->getPathBase()+argname;
		GenMap map=getVarList(path);
		std::vector<GenString> vect;
		IODatav03 *tm=defitem->getModel();
		for(GenMapProto::Iterator it: map){
			GenString p=it.key();
	//		std::cout << "getArgumentList:" << p <<", "<< it.value() << std::endl;
			GenString npath=defitem->getModelFullPath()+'/'+p;
			unsigned int fps=defitem->getModelFullPath().size();
	//		bool exist=tm->hasPath(npath);
			GenString value=tm->get(npath);
			if(!value.empty()) {vect.push_back(npath.substr(fps+1));continue;}
			GenString progpath=tm->progeny(npath);
			while(!progpath.empty()){
				vect.push_back(progpath.substr(fps+1));
				progpath=tm->progeny(npath,progpath);
			}
		}
		return vect;
	//	get
	}

	GenMap getVarList(GenString &varpath){
		//	bool b=false;
		GenMap varvals;
		std::vector<GenString> keys = getDefs(varpath); // get all variable names defined under varpath
		for(GenString k:keys){
			std::vector<GenString> subkeys =getDefs(varpath+"/"+k); //
			//			 	std::cout << "key :"<< (varpath+'/'+k) << " - key numbner : " << subkeys.size() << std::endl;
			if(subkeys.size()>1){	// the variable is an array with sub objects
				for(GenString sk:subkeys){// if multiple subvalues, means it has subnodes
					//		 			std::cout << "subkey :"<< sk << std::endl;
					GenString subkeyval =getDefVal(varpath+"/"+k+"/"+sk);
					//		 			std::cout << "subkey val :"<< subkeyval << std::endl;
					//conformValue(k+"/"+sk,subkeyval,defitem);
					varvals.set(k+"/"+sk,subkeyval);
				}

			} else {
				GenString subval;
				//			if(subkeys.size()>0) std::cout << "subsubchildren: " << getDefs(varpath+'/'+k+'/'+subkeys.front()).size() << std::endl;
				if(subkeys.size()==1){
					GenString subvarval=varpath+'/'+k+'/'+subkeys.front();
					if(getDefs(subvarval).size() >0){//subkey has children
						subval =getDefVal(subvarval);
						k=subkeys.front();
						// this finding the value at the leaf of the tree
						// maybe should be done by getDefVal and return a defval whatever path we give, if several are found, just take first (0 if numeral)
					}
				}
				if(subval.empty()){//subkey has no children or several, not one only
					subval=getDefVal(varpath+'/'+k);
					if(testNumeral(k)) {k=subval;subval="";}// if a numeral key, it means the subval is the var name
				}


				varvals.set(k, subval);
				//conformValue(k,subval,defitem);
			}
		}
		return varvals;
	};


	/*	ConformVar function
	 * conform all variables found in the specified varpath to the definition.
	 * return true if a value was changed (by conformvalue)
	 * */
	bool conformVar(GenString &varpath, DefItem *defitem){
		bool b=false;
		//	if(varpath=="actions/selectassign/args"){std::cout << "stop here" << std::endl;}
		GenMap varvals=getVarList(varpath);
		//	std::cout << "conformVar : varpath : "<< varpath << " - varlist size : " << varvals.size() << std::endl;
		for(auto it:varvals){
			GenString k=it.key(), v=it.value();
			//		std::cout << "conformVar : key :"<< k << " - value : " << v << std::endl;
			if(conformValue(k, v,defitem)) b=true;
		}
		return b;
	}


	bool conform(GenString vardefpath, DefItem *defitem){
		std::vector<GenString> vect=defitem->conformPaths(vardefpath);
		for(GenString p:vect){
			conformVar(p,defitem);
		}
		return true;
	}



	bool subscribeValue(GenString varmodelrelpath, DefItem *defitem){
		varmodelrelpath=defitem->getModelFullPath()+"/"+varmodelrelpath;
//		std::cout << "OperationDefs:: subscribed : "<<varmodelrelpath << std::endl;
		defitem->getModel()->on(varmodelrelpath, RF("write"), defitem->getListener());
		//oplib->getModel()->subscribeToPath(varmodelrelpath,listener);
		//varmodelpath=defitem->getModelFullPath()+"/"+varmodelpath;
		return true;
	}

	bool subscribeVar(GenString &vardefpath, DefItem *defitem){

		bool b=false;
		GenMap varvals=getVarList(vardefpath);
		//		std::cout << "subscribeVar : varpath : "<< vardefpath << " - varlist size : " << varvals.size() << std::endl;
		for(auto it:varvals){
			GenString k=it.key();
			//			std::cout << "subscribeVar : key :"<< k << std::endl;
			if(subscribeValue(k, defitem)) b=true;
		}
		return b;

	}


	bool subscribe(GenString vardefpath, DefItem *defitem){//GenString basemodelpath, IODatav03 *model, IODataListener *listener){
		std::vector<GenString> vect=defitem->subscribePaths(vardefpath);
		for(GenString p:vect){
			subscribeVar(p,defitem);
		}
		return true;
	}


};


#endif



