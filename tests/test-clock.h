
#include "../infrastructure/SyncedClock.h"


bool testClock(){
	SyncedClock CLOCK(32);
	uint64_t mss=millis64();
	CLOCK.resyncMS(mss );
	uint64_t mss1=CLOCK.getMS();
	uint64_t diff0=mss1-mss;
	while (1) {
		uint64_t ms0=millis64();
		uint64_t ms1=CLOCK.getMS();
		uint64_t diff=ms1-ms0;
		if(ms0>ms1) diff=ms0-ms1;
		println(GenString()+"millis: "+to_string(ms0)+" diff:"+to_string(diff));

		println(GenString()+"getMS: "+to_string(ms1)+" diff:"+to_string(diff));
		delay(1000);
	}

	return true;
};



