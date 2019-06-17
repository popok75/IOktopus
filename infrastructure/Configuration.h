#ifndef CONFIGURATION_H
#define CONFIGURATION_H


#include "CompatFS.h"
#include "../datastruct/GenMap.h"


#undef FTEMPLATE
#define FTEMPLATE ".irom.text.configuration1"

#include "../datastruct/GenMap.h"

#include "CompatFS.h"

bool parseTxtMap(GenString content, GenMap &dest);

class Configuration {
public:
	GenMap configmap;

	//manage reconfiguration as well

	bool loadTxtFile(GenString filename){
		//#define FTEMPLATE ".irom.text.configuration3"
		println(GenString()+RF("Configuration : loadTxtFile loading ")+filename);
		size_t sz;
		unsigned char *buff=CURFS.readFileBuffer(filename,sz);
		if(!buff) {
			//println(GenString()+RF("Configuration : loadTxtFile file not found ")+filename);
			return false; // no setup file, stop there
		}

		std::string str((const char *)buff,sz);
		delete buff;

		bool b=parseTxtMap(str, configmap);
		//	str.find(RF("\n"),0); //sram use 256
		println(GenString()+RF("Configuration : parsed config :")+configmap.asJson());
		return b;
	}

};

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.configuration2"


void eraseComments(std::string &json,unsigned int index1,unsigned int index2){

	unsigned index=json.find("/*",index1),index2c=index2;

	while(index<index2c){
		unsigned int eindex=json.find("*/",index+2);
		if(eindex<index2c) {
			unsigned int cl=2+eindex-index;
			json.erase(index,cl);
			index2c-=cl;
		} // if not true there is an error
		index=json.find("/*",index1);
	}

	index=json.find("//",index1);	// find starting comment
	//	StringType str;
	//	if(index<json.size()) str=json.substr(index,100);
	while(index<index2c){
		unsigned int eindex=json.find("\n",index+2);	// find end of comment

		if(eindex>=index2c) eindex=index2c;
		unsigned int cl=eindex-index;// do not erase the next \n
/*		println("erasing '"+json.substr(index,cl)+"'");
		char c=json[eindex];
		char c2=json[eindex-1];
		println("after '"+json.substr(eindex,20)+"'");*/
//		println("size: "+to_string(json.size())+", index2c:"+to_string(index2c));
		index2c-=cl;
		json.erase(index,cl);

		index=json.find("//",index1);
	}
	replaceAll(json,"\r\n","\n");
	replaceAll(json,"\n\n","\n");

};

void eraseComments(GenString &content){
	eraseComments(content,0,content.size());
}

bool parseTxtMap(GenString content, GenMap &destmap){

	eraseComments(content);



	unsigned int ip=0, i=content.find(RF("\n"),ip);
	while(i<content.size()){
		// for each non void line
		GenString sub=content.substr(ip,i-ip);
		ip=i+1;
		i=content.find(RF("\n"),ip);

		unsigned int commi=sub.find(RF("//"));
		if(commi<sub.size()) sub=sub.erase(commi);
		sub=replaceAll(sub,RF("\r"),"");
		sub=replaceAll(sub,RF("\t"),RF(" "));
		while(sub[0]==' ') sub=sub.substr(1);
		while(sub.size()>0 && sub[sub.size()-1]==' ') sub=sub.substr(0,sub.size()-1);

		if(sub.size()==0) continue;
		// split at :
		// store in map
		unsigned int j=sub.find(RF(":"),0);
		if(j>=sub.size()) j=sub.find(RF("="),0);
		if(j<sub.size()){
			GenString pre=sub.substr(0,j), post=sub.substr(j+1);
			// lower case left, remove spaces from edges
			while(pre[pre.size()-1]==' ') pre=pre.substr(0,pre.size()-1);
			for(auto& c : pre) c = tolower(c);
			while(post[0]==' ') post=post.substr(1);
			// store in map
			//	pre.shrink_to_fit();post.shrink_to_fit();
			destmap.set(pre,post);

		} else {
			println("Configuration::parseTxtMap: error : "+sub);
			return false;
		}
	}

	return true;
};
//#define FTEMPLATE ".irom.text.configuration4"
#endif
