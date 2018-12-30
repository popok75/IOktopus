#ifndef GENSTRING_H
#define GENSTRING_H


#include <string>
#define GenString std::string



uint64_t stoull(std::string const& value) {
	uint64_t result = 0;

	char const* p = value.c_str();
	char const* q = p + value.size();
	while (p < q) {
		result = (result << 1) + (result << 3) + *(p++) - '0';
	}
	return result;
}

std::string replaceAll(std::string src, std::string pattern,std::string newval){
	std::string::size_type n = 0;
	while ( ( n = src.find( pattern, n ) ) != std::string::npos )
	{
		src.replace( n, pattern.size(), newval );
		n += pattern.size();
	}
	return src;
}

bool endsWith(std::string const &fullString, std::string const &ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
	} else
		return false;
};

bool startsWith(std::string const &fullString, std::string const &start) {
	if (fullString.length() >= start.length()) {
		return (0 == fullString.compare (0, start.length(), start));
	} else
		return false;
};



bool isDigit(GenString str){
	char *ptr;
	//double srcval=
	std::strtod(str.c_str(),&ptr);
	if(*ptr || str.size()==0) return false;
	return true;
};

double strToDouble(std::string str){
	char *ptr;
	double srcval=std::strtod(str.c_str(),&ptr);
	//	if(*ptr || str.size()==0) return false;
	return srcval;
};

unsigned long strToUnsignedLong(std::string value){

	//	unsigned long srcval=std::stoul (str);
	//	return srcval;
	uint64_t result = 0;
	char const* p = value.c_str();
	char const* q = p + value.size();
	while (p < q) {
		result = (result << 1) + (result << 3) + *(p++) - '0';
	}
	return result;
};

GenString getMin(GenString v0,GenString v1){
	if(isDigit(v0)){
		if(isDigit(v1)){
			double d0=strToDouble(v0);
			double d1=strToDouble(v1);
			if(d0<d1) return v0;
			else return v1;
		} else return v0;
	} else if(isDigit(v1)) return v1;
	else return "";
};

GenString getMax(GenString v0,GenString v1){
	if(isDigit(v0)){
		if(isDigit(v1)){
			double d0=strToDouble(v0);
			double d1=strToDouble(v1);
			if(d0>d1) return v0;
			else return v1;
		} else return v0;
	} else if(isDigit(v1)) return v1;
	else return "";
};

GenString getPathLeaf(GenString path){
	while(path[path.length()-1]=='/') path.erase(path.length()-1);
	unsigned int i=path.rfind('/');
	return path.substr(i+1);

};
GenString getPathBranch(GenString path){
	while(path[path.length()-1]=='/') path.erase(path.length()-1);
	unsigned int i=path.rfind('/');
	if(i==0) return "";
	return path.substr(0,i);

};

std::string to_string(uint64_t num){
	uint8_t i = 0;  uint64_t n = num;
	char str[24];//should be 21
	do i++;
	while ( n /= 10 );

	str[i] = '\0';
	n = num;

	do str[--i] = ( n % 10 ) + '0';
	while ( n /= 10 );

	return std::string(str);
};

#include <math.h>
std::string to_stringWithPrecision(double d,int decimals){
	bool neg=false;
	if(d<0) {neg=true;d=-d;}
	long int l=1;
	for(int i=decimals;i>0;i--) l=l*10;
	l=round(l*d);
	std::string str=to_string(l);
	int sl=str.size();
	while(sl<=decimals) {str="0"+str;sl=str.size();}
	str=str.substr(0,sl-decimals)+"."+str.substr(sl-decimals,sl);
	for(int i=decimals;i>0;i--)
		if(str.substr(str.size()-1,str.size())=="0")
			str=str.substr(0,str.size()-1);
	if(str.substr(str.size()-1,str.size())==".") str=str.substr(0,str.size()-1);
	if(neg && str!="0") str="-"+str;
	return str;
}


/*
////////////
///Memory tests:

//////////// math.h : 0 bytes sram
#include <math.h>
void setup() {
   int u,l=round(u);
}
void loop() { delay(100);}
////////////

//////////// string : 416 bytes sram
#include<string>
#define RF(x) String(F(x)).c_str()
void setup() {
    Serial.begin(115200);
 // int u,l=round(u);
  std::string str,str2;
  int n,n1;
   str+=str;
  if(str==str2) str2;
  str.find(str2,n);
  str.compare(n1,n,str2);
  str.replace(n,n1,str2);
  str.size();

  str.c_str();
  str.erase(n);
  str.rfind(str2);
  str.substr(n,n1);

  std::strtod(str.c_str(),0);
  Serial.print(("1234567"));
}
void loop() {delay(100);}
////////////
*/

#endif
