
#include "../modules/datamodel/IODatav03/BasicIODataKit.h"


IODatav03 *testdatamodel;

bool testIODatav03(){

	CLOCK32.resyncSec(millis64()/1000);	//needed on x86 to have the correct TS


	GenString actionfile="datafiles/action_defs.json";
	GenString opfile="datafiles/operation_defs.json";
	GenString opdatafile="datafiles/operation_data.json";
	testdatamodel=BasicIODataKit::setupModel(opdatafile,opfile,actionfile);

	testdatamodel->updateVal("/operations/Timer/active","1");


	return false;
};












