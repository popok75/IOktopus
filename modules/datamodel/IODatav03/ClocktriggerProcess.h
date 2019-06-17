#include "OperationLib.h"

/*	This file is for the ClocktriggerProcess. A bit complex class :
 * 				- triggering a timer when 'start' is modified, or when 'step' is modified
 * 				- according to an array of 'durations',
 * 				- will increment a 'step' variable
 * 				- when possible will also write a ts0 matching the theoric start time from step 0
 * 				- if step or current duration is not a number, will stop the timer (maybe should self disable)
 * */

#include "../../../infrastructure/SyncedClock.h"

#define CLOCKTRIGGERPROCESSNAME "clocktrigger"
#define ARGSBEFOREDURATIONS 3

class ClocktriggerProcess  : public BasicActionProcess {
	Ticker processticker;

	bool timerstarted=false;
	unsigned int savedstep=0;
	uint64_t savedts0=0;
	std::vector<unsigned> saveddurations;
	unsigned int savedcurrentduration;

public:
	virtual ~ClocktriggerProcess(){};
	ActionProcess* duplicate(){return new ClocktriggerProcess();};

	static void statictick(ClocktriggerProcess*ptr){if(ptr) ptr->tick();}
	void startTimer(unsigned dur ){
		// check it is not already started
		if(timerstarted) return;
		timerstarted=true;
		processticker.once_ms(dur, statictick,this);
	}
	void stopTimer(){
		if(!timerstarted) return;
		processticker.detach();
		timerstarted=false;
	}

	void updateTS(const std::vector<GenString> &args, const GenString &tspath, unsigned step, unsigned currentstepprogress=0){
		unsigned pstep=step;
		uint64_t pastdur=currentstepprogress;
		saveddurations.clear();
		while(pstep>0) {			// count past time from current step
			pstep--;
			GenString pdur=getArgumentValue(args[pstep+ARGSBEFOREDURATIONS]);
			if(!durationFormat(pdur)) {saveddurations.clear();
			if(getArgumentValue(tspath)!="") updateArgumentValue(tspath,"");
			return ;	// error, one of prev duration is not convertible to MS, cannot update TS
			}
			unsigned updur=durationToMS(pdur);
			pastdur+=updur;
			saveddurations.push_back(updur);
		}
		uint64_t ts0=CLOCK32.getMS();
		//		std::cout << "now:" << ts0 <<std::endl;
		//		std::cout << "pastdur:" << pastdur <<std::endl;
		ts0-=pastdur;
		savedts0=ts0;
		ts0=ts0/1000;	//convert to seconds
		savedstep=step;
		GenString ts0str=to_string(ts0);
		// this info is ts0, but recalculated and possibly updated each step
		if(getArgumentValue(tspath)!=ts0str) updateArgumentValue(tspath,ts0str);		// update ts0, skip if it is same value
		//		std::cout << "TS:" << ts0str <<std::endl;
	}

	uint64_t getProgressMS(){
		uint64_t diff=CLOCK32.getMS()-savedts0;
		for(unsigned int d : saveddurations) diff-=d;
		return diff;
	}

	void startNextStep(bool incrementstep=false, bool keepprogress=false){
		std::vector<GenString> args=getArgumentNames(RF("args"));
		auto results=getArgumentNames(RF("results"));
		GenString steppath=args[2];
		GenString cyc=args[1];
		GenString stepstr=getArgumentValue(steppath);
		GenString cycstr=getArgumentValue(args[1]);

		if(!isDigit(stepstr)) {stopTimer();return;}		//error
		unsigned step=strToUint64(stepstr);
		if(incrementstep) step++;
		std::cout << "ClocktriggerProcess:: start next step :" << step <<std::endl;

		if(cycstr=="1") step=step%(args.size()-ARGSBEFOREDURATIONS);	//if cycle, restart at 0
		else if(step>(args.size()-ARGSBEFOREDURATIONS)) {stopTimer();return ;}		//error

		GenString curdur=getArgumentValue(args[step+ARGSBEFOREDURATIONS]);	// test for duration format ?
		unsigned cdur=durationToMS(curdur);
		savedcurrentduration=cdur;

		if(keepprogress) {
			unsigned progress=getProgressMS();
			if(cdur<progress) {	//if too small just go to next step, don't keep any additional progress, could go on to next steps until all progress is spent
				step++;
				curdur=getArgumentValue(args[step+ARGSBEFOREDURATIONS]);
				cdur=durationToMS(curdur);
				savedcurrentduration=cdur;
			} else cdur-=progress;
		}
		startTimer(cdur);

		savedstep=step;
		updateArgumentValue(steppath,to_string(step));				// update step

		updateTS(args,results[0],step);
	}


	void tick(){
		// get the step,
		// increment it,
		// start next timer
		std::cout << "timer working" <<std::endl;
		react=false;	// allow change of input arguments without reacting
		timerstarted=false;
		startNextStep(true);
		react=true;
	}

	// how it works :
	// 1. when started, it will start at the available step and update ts0 (in future will use ts0 if future date to planned start)
	// 2. at the end of interval, it will increment step, eventually update ts0
	// 3. if cyc=1, and no more durations are available, it will start again from step 0 and update ts0. If cyc!=1 it will stop
	// 4. if during a cycle, step is modified, it will (re-)start at the new step, ts0 will be updated accordingly
	// 5. if during cycle, durations are modified
	//			- Durations after current step will not affect functioning (until they become current step)
	//			- Durations before current step will update ts0.
	//			- Current step duration will trigger a reschedule/skip of current timer to adapt to the new interval, while keeping it's progress.
	//				e.g. duration 20 half elapsed changed to 30 will start next step 30-(20/2)=20 seconds.
	//				e.g. duration 40 half elapsed changed to 10 will start next step immediately 10-(40/2)=-10 seconds.


	bool prevDurationsChange(const std::vector<GenString> &args){
		unsigned pstep=savedstep;
		while(pstep>0) {			// count past time from current step
			pstep--;
			GenString pdur=getArgumentValue(args[pstep+ARGSBEFOREDURATIONS]);
			if(!durationFormat(pdur)) {
				return true;	// error, one of prev duration is not convertible to MS, cannot update TS
			}
			unsigned updur=durationToMS(pdur);
			if(saveddurations[savedstep-1-pstep]!=updur) return true;
		}
		return false;
	}

	virtual void disable(){stopTimer();}

	virtual void basicprocess(std::vector<GenString> args,std::vector<GenString> results,GenString path, Event *e){
		if(!inited) {init(); return;}
		if(!react) return;	//avoid self notification	// could be moved to ActionProcess::updateArgumentValue

		unsigned extraargs=ARGSBEFOREDURATIONS;	//before durations
		if(args.size()<(extraargs+1)) return;				// not enough arguments
		GenString startstop=args.front();
		GenString steppath=args[2];
		if(!timerstarted && getArgumentValue(startstop)=="1") {
			startNextStep();
			return;
		}
		if(timerstarted && getArgumentValue(startstop)=="0") {
			stopTimer();
			return;
		}
		if(timerstarted && getArgumentValue(startstop)=="1") {
			GenString stepstr=getArgumentValue(steppath);
			if(isDigit(stepstr)){
				if(strToUint64(stepstr)!=savedstep) {				// change of step
					stopTimer();
					startNextStep();	// we could ask a change of step with keeping progress if relevant
					return;
				} else if(durationFormat(args[savedstep+ARGSBEFOREDURATIONS])
						&& strToUint64(args[savedstep+ARGSBEFOREDURATIONS])!=savedcurrentduration) {	//change in current step duration
					startNextStep(false, true);	// keep current progress
					return;
				} else {
					if(prevDurationsChange(args)) updateTS(args,results[0],savedstep, getProgressMS());// see if there is a change in durations
				}
			} //else {stopTimer();return;}	// if step is not valid, do nothing (will stop by itself) or stop timer ?
		}


	};	// each notification goes here
	virtual bool named(GenString name){if(name==RF(CLOCKTRIGGERPROCESSNAME)) return true; else return false;};
};


















