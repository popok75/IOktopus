
#include <map>
#include <iostream>
#include <vector>

#define x86BUILD
#define RF(x) x

#include "infrastructure/x86/monoTimer.h"


#include "IOktopus.ino"


void tests();
int main(int argc, char* argv[])
{
	std::cout << "IOktopusx86 v0.3 "<<std::endl;



	setup();

	// x86 timer and server maintaining

	bool first=true;
	while(1){
		if(first) {std::cout << "loop started " << monomillis() << std::endl;first=false;}
		//or IOFactory::yield();
		loop();
		//	 std::cout << ". " << monomillis() << std::endl;
		delay(100);
		//		*	std::cout << "millis 2 " << monomillis() << std::endl;
		//	std::cout << std::endl;
	}

	std::cout << "IOktopusx86 v0.01 ended"<<std::endl;

	//end();	//cleanup

	return 0;
}


/*
#include "tests/test-datastruct.h"
//#include "tests/test-clock.h"

#include "tests/test-iodatav03.h"
void tests(){

	testIODatav03();


	//	testChunkedVector();

	/ *
		bool b=testObjMap();
		if(!b) {std::cout << "testObjMap failed "<<std::endl;return -1;}


		b=testMap();
		if(!b) {std::cout << "testMap failed "<<std::endl;return -1;}
		b=testTreeMap();
		if(!b) {std::cout << "testTreeMap failed "<<std::endl;return -1;}
	 */
	//	b=testClock();
	//	if(!b) {std::cout << "testClock failed "<<std::endl;return -1;}


	/*
		b=testCurFS();
		if(!b) {std::cout << "testCurFS failed "<<std::endl;return -1;}
	 * /

	//	testSSMap();
	bool first=true;
	while(1){
		if(first) {std::cout << "loop started " << monomillis() << std::endl;first=false;}
		//or IOFactory::yield();

		//	 std::cout << ". " << monomillis() << std::endl;
		delay(100);
		//		*	std::cout << "millis 2 " << monomillis() << std::endl;
		//	std::cout << std::endl;
	}
}
*/
