
#include"../datastruct/GenMap.h"
#include"../infrastructure/CompatPrint.h"

#include"../datastruct/ChunkedVector.h"

bool testChunkedVector(){



//	GenSSMap map={{"type","input"}, {"unit","\%"}, {"val","--"}};

	ChunkedVector<uint64_t> vect(32);

	vect.push_back(0);
	vect.push_back(10);
	vect.push_back(200);
	vect.push_back(3000);
	vect.push_back(40000);
	vect.push_back(500000);
	vect.push_back(600000);
	vect.push_back(700000);
	vect.push_back(800000);
	vect.push_back(900000);

	println(GenString()+"vect size:"+to_string(vect.size()));
	for(unsigned int i=0;i<vect.size();i++){
		println("preget");
		uint64_t tt=vect.get(i);
		println("preprint");
		println(GenString()+"vect["+to_string(i)+"]:"+to_string(tt));
		println("postprint");
	}
/*
	vect.erase(1,3);

	println(GenString()+"vect size:"+to_string(vect.size()));
	for(unsigned int i=0;i<vect.size();i++){
		println(GenString()+"vect["+to_string(i)+"]:"+to_string(vect[i]));
	}

	vect.insert(2,999);
	println(GenString()+"vect size:"+to_string(vect.size()));
	for(unsigned int i=0;i<vect.size();i++){
		println(GenString()+"vect["+to_string(i)+"]:"+to_string(vect[i]));
	}
	vect.insert(0,666);

	println(GenString()+"vect size:"+to_string(vect.size()));
	for(unsigned int i=0;i<vect.size();i++){
		println(GenString()+"vect["+to_string(i)+"]:"+to_string(vect[i]));
	}
*/
	while(!vect.empty()){
		vect.erase(0);
		println(GenString()+"vect size:"+to_string(vect.size()));
		for(unsigned int i=0;i<vect.size();i++){
			println("preget");
			uint64_t tt=vect.get(i);
			println("pretostringtt");
			GenString stt=to_string(tt);
			println("pretostringsi");
			GenString si=to_string(i);
			println("preconcat");
			GenString cc=GenString()+"vect["+si+"]:"+stt;
			println("preprint");
			std::cout<< cc <<std::endl;
			println("postprint");
	//		println(GenString()+"vect["+to_string(i)+"]:"+to_string(vect[i]));
		}

	}

	return true;
};


bool testSSMap(){

	GenSSMap map={{"type","input"}, {"unit","\%"}, {"value","--"}};
	// check embraced creation
	GenString type=map.get("type");
	bool b=(map.get("type")=="input") && (map.get("unit")=="\%") && (map.get("val")=="--");
	if(!b) return false;

	// try update of key
	// check update of key

	return true;
};

bool testMap(){

	GenMap map={{"type","input"}, {"unit","\%"}, {"value","--"}};
	// check embraced creation
	GenString type=map.get("type");
	bool b=(map.get("type")=="input") && (map.get("unit")=="\%") && (map.get("val")=="--");
	if(!b) return false;

	// try update of key
	// check update of key

	return true;
};

/*
bool testTreeMap(){
	GenMap map={{"type","input"}, {"unit","\%"}, {"value","--"}};
	GenString path="/nodes/Humidity";
	GenTreeMap treemap;
	treemap.update(path,map);

	bool b=(treemap.get(path+"/type")=="input") && (treemap.get(path+"/unit")=="\%") && (treemap.get(path+"/value")=="--");
	if(!b) return false;

	return true;
};




bool testObjMap(){
	// test with genmap and setObj with pointer
	GenMap map;
	struct Obj {
		int a=52;
		std::string str;
		bool operator == (const Obj &o) const
				  { return a==o.a && str==o.str; }
	} o, *o2, o3;
	o.a=53;
	o.str="hello";
	map.setObj("obj",&o);
	o2=map.getObj<Obj*>("obj");
	o2++;
	// test genobjmap
	GenObjMap<GenString,Obj> objmap;
	objmap.set("obj",o);
	o3=objmap.get("obj");


	return true;
}
*/
