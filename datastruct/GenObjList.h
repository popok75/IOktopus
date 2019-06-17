#ifndef GENOBJLIST_H
#define GENOBJLIST_H

#include<vector>

template <typename TVal>
class GenListProto{

};

// desirable functions : load from std::vector, convert into std::vector


template <typename TVal>
class GenListStl: public GenListProto<TVal>{
	std::vector data;

};



#define GenObjList std::vector //GenObjListProtoStlVector

void func(){

	GenObjList<int> obj;


}


#endif

