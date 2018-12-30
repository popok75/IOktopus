//#include<string>
#ifndef PRINTESP8266
#define PRINTESP8266

/*
template <typename T>
std::string to_string(T i){
	String str(i);
	return std::string(str.c_str());
};
*/

float stof(std::string str) {
	return String(str.c_str()).toFloat();
}

//int myinit=
void initPrint(){Serial.begin(115200);}
void print(std::string str){Serial.print(str.c_str());}
void println(std::string str){Serial.println(str.c_str());}
void println(){Serial.println();}
void println(uint64_t str){Serial.println(to_string(str).c_str());}
void println(int64_t str){Serial.println(to_string(str).c_str());}
//template <typename T> void mprint(T str){Serial.print(str);}
template <typename T> void print(T str){Serial.print(str);}
template <typename T> void println(T str){Serial.println(str);}


template <typename T,typename T2> void println(T str, T2 i){Serial.println(str,i);}

/// serial : 164bytes SRAM
/*void setup() {
  Serial.begin(115200);
  Serial.println(F("1234567"));
}
void loop() { }
*/

#endif
