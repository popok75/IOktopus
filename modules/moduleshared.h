

#ifndef IOKTOPUS_MODULES_MODULESHARED_H_
#define IOKTOPUS_MODULES_MODULESHARED_H_

// following list used by data only ? if yes move there
#define VALUE_FIELD "value"					// used in log, in basickit for min max calculation, and in device(BasicReader)  to provide value
#define TIMESTAMP_FIELD "timestamp"			// used in log and in device (BasicReader)
#define MIN_FIELD "min"						// used in the basic iodatakit
#define MAX_FIELD "max"						// used in the basic iodatakit
#define TAG_FIELD "tag"						// used in log16
#define DISCRETE_TAG "discrete"				// used in log16


#define UNIT_FIELD "unit"					// used in device(BasicReader) at init
#define TYPE_FIELD "type"					// used in device(BasicReader)  at init
#define INPUT_TYPE_VALUE "input"			// used in device(BasicReader)  at init

#define MODEL_UPDATED_EVENT "modelUpdated"	// notify change of model	// used by data to notify logger
#define	UPDATE_MODEL_EVENT "updateModel"		// write in model		// used by device and server to write data
#define GET_JSON_DATA_EVENT "getAsJson"									// used by server to get data in json format
#define GET_DATA_SUBVALUES_EVENT "getDataSubValues"						// used by logger to read any path in map format
//#define GET_DATA_NODE_EVENT "getDataNode"								// used by logger to read a specific node in map format
//#define GET_DATA_VALUE_EVENT "getDataValue"								// used by logger to read a specific value in string format
#define GET_JSON_LOG_EVENT "getLogJson"									// used by server to get log data


#endif /* IOKTOPUS_MODULES_MODULESHARED_H_ */
