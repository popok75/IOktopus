#ifndef MONOTIMER_H
#define MONOTIMER_H

#include<vector>
#include<functional>
#include<future>


/*
	MonoTimer is a procedural monothread task scheduler that emulate timers on esp8266
 		- tasks once or forever can be scheduled using monoSetTimeOut (& cancelled using monoCancelTimeOut)
 		- the scheduler can be run using yield & delay

 	MonoTimer runs as following : tasks are piled and the scheduler wait (or not) untill the due time comes, then start the task,
		- monoRunOnce :
				- loop on tasks : execute the ones who has (just) past due time, then remove or renew them
					- if cant wait stop after only one execution
				- if can wait, find next due time and wait until time passes or a new task is added

 	MonoTimer is thread safe as mutex is set while modifying the task list

 * */


static inline
uint32_t monomillis(){
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
};

static inline
uint64_t monomillis64(){
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
};

static uint64_t monoidstat=1;

struct TaskStruct {
	uint64_t id;
	uint32_t due;
	unsigned int interval;
	std::function<void (void)> f;
public:
	TaskStruct(uint32_t due0, std::function<void (void)>f0,unsigned int interval0=0): f(f0){
		due=due0; interval=interval0;id=monoidstat++;	// id is not really thread safe
	//	std::cerr << "TaskStruct created id:"<<id<<std::endl;
	};

};








static std::timed_mutex createmutex;
static std::mutex tasksmutex, waitmutex;
static bool monowait=false,monorestart=false;
static std::vector<TaskStruct*> monotasks;
static bool monorun=true;


/*
inline
uint32_t monomillis(){
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
};
 */


static
void monoCancelTimeOut(TaskStruct*ts){
	for(unsigned int i=0;i<monotasks.size();i++){
		tasksmutex.lock();
		if(monotasks[i]->id==ts->id) {
//			std::cout << "monoRun cancelling... "<< std::to_string(ts->id) << std::endl;
			monotasks.erase(monotasks.begin()+i);
			delete ts;
			monorestart=true;
			tasksmutex.unlock();
			return;
		};
		tasksmutex.unlock();
	}
};


void printTasks(){
	for(unsigned int i=0;i<monotasks.size();i++){
		std::cout << "printTasks: id:"  << std::to_string(monotasks[i]->id)
		<< ", due:"<<std::to_string(monotasks[i]->due)
		<< ", f:"<< std::to_string((uint64_t)&(monotasks[i]->f))  << std::endl;
	}
}


template <typename... T>
TaskStruct*  basemonoSetTimeOut(unsigned int ms, bool repeat, void(fc)(T...),T... params){
	// std::function<void (T...) >

	//std::function<void (int)>

	std::function<void (void)> ffc=std::bind(fc,params...);

	TaskStruct *ts=new TaskStruct(monomillis()+ms, ffc);
	if(repeat) ts->interval=ms;
	tasksmutex.lock();
	//	printTasks();
	monotasks.push_back((ts));
//	std::cout << "monosetTimeOut:added task id: "  << std::to_string(ts->id) << std::endl;
	//	printTasks();
	tasksmutex.unlock();
	//	std::cout << "monosetTimeOut:unlocking... "  << std::endl;
	waitmutex.lock();
	if(monowait) {//std::cout << "monosetTimeOut:closed lock... "  << std::endl;
		createmutex.unlock();
	}
	waitmutex.unlock();
	//	else {std::cout << "monosetTimeOut:open lock... "  << std::endl;}
	return ts;
	//	std::cout << "monosetTimeOut:unlocked... "  << std::endl;
};


template <typename... T>
TaskStruct*  monoSetTimeOut(unsigned int ms, bool repeat, void(fc)(T...),T... params){
	return basemonoSetTimeOut(ms,repeat,fc,params...);}

TaskStruct*  monoSetTimeOut(unsigned int ms, bool repeat, void(fc)()){
	return basemonoSetTimeOut(ms,repeat,fc );};

TaskStruct*  monoSetTimeOut(unsigned int ms, bool repeat, void(fc)(int),int i){
	return basemonoSetTimeOut(ms,repeat,fc,i );};

TaskStruct*  monoSetTimeOut(unsigned int ms, bool repeat, void(fc)(int,std::string),int i,std::string str){
	return basemonoSetTimeOut(ms,repeat,fc,i,str );};



TaskStruct* monoFind(uint64_t id){
	tasksmutex.lock();
	for(unsigned int j=0;j<monotasks.size();j++){
		if(id==monotasks[j]->id) {
			tasksmutex.unlock();
			return monotasks[j];
		}
	}
	tasksmutex.unlock();
	return 0;
};


static
void monoRunOnce(bool canwait, unsigned int waitms=0){
	//std::cout << "monoRun loop1... " << std::endl;
	unsigned int deadline=monomillis()+waitms;

	monorestart=false;

	for(unsigned int i=0;i<monotasks.size();){ // loop on tasks : execute the ones who has (just) past due time, then remove or renew them
		unsigned int m=monomillis();
		//std::cout << "monoRun loop2... " << std::endl;
		TaskStruct*ts=monotasks[i];
		uint64_t id=ts->id;
		if(ts->due<=m) {
			std::function<void (void)> fc=ts->f;
			if(ts->interval>0) {
			//	std::cout << "monoTimer::executing renewing:"  << std::to_string(id)<< std::endl;
				ts->due+=ts->interval;
			} else {
			//	std::cout << "monoTimer::deleting... "<< std::to_string(id) << std::endl;
				tasksmutex.lock();
				for(unsigned int j=0;j<monotasks.size();j++){	// in case it has moved since we got i
					if(id==monotasks[j]->id) {
						monotasks.erase(monotasks.begin()+j);
						delete ts;
						break;
					}
				}
				tasksmutex.unlock();
			}
		//	std::cout << "monoTimer::executing task:"  << std::to_string(id)<< std::endl;
			fc();
		//	std::cout << "monoTimer::executed task:"  << std::to_string(id)<< std::endl;

			if(!canwait) break;	// do one execution only
		}
		if(monorestart){monorestart=false;i=0;}	// isn't there a risk of perpetual loop here ?
		else i++;
	}

	if(canwait) {	// if can wait, find next due time and wait until time passes or a new task is added
		unsigned int tsize=monotasks.size();
		unsigned int mintime;
		bool foundmin=false;
		for(unsigned int i=0;i<monotasks.size();i++){	// find next due task time
			//std::cout << "monoRun loop2... " << std::endl;
			TaskStruct*ts=monotasks[i];
			if(!foundmin) {foundmin=true;mintime=ts->due; continue;}
			if(ts->due<mintime) {mintime=ts->due;}
		}
		if(waitms && deadline<mintime) mintime=deadline;	// we stop at deadline if before next task

		if(foundmin && tsize==monotasks.size() && (mintime>monomillis())){	// be sure that we didnt add any timeout since the loop
			//	println("monoRun:locking mutex");
			waitmutex.lock();
			createmutex.lock();
			monowait=true;
			waitmutex.unlock();
			//	println("monoRun:locking mutex2");
			//		std::cout << "monoRunOnce 1 " << monomillis() << " "<<mintime<<std::endl;
			createmutex.try_lock_for(std::chrono::milliseconds(mintime-monomillis())); // wait until next task(b=false) or a mutex unlock (b=true)
			//			if(!b) {println("monoRun:mutex timed out");}
			//			std::cout << "monoRunOnce 2 " << monomillis() << std::endl;
			waitmutex.lock();
			createmutex.unlock();
			monowait=false;
			waitmutex.unlock();
			//	println("monoRun:unlocked mutex");
		} else std::this_thread::yield();
	} else std::this_thread::yield();
	//	std::this_thread::sleep_for(std::chrono::milliseconds(1));	// could wait the right time if no other thread adds timeout

};


void monoRun(){
	monorun=true;
	while(monorun) monoRunOnce(true);
}


static
void monoRunFor(unsigned int ms){
	unsigned int msstop=monomillis()+ms;
	while(msstop>monomillis()) monoRunOnce(true, msstop-monomillis());
}


static
void delay(int ms){monoRunFor(ms);}


void yield(){monoRunOnce(false);}


inline uint32_t millis(){
	return monomillis();
};


/*
unsigned int count=0;
void tickstatic(int i){
	std::cout << "Tic... " <<count++<< std::endl;
	std::cout << "Millis... " <<millis()<< std::endl;
	std::cout << "i... " <<i++<< std::endl;
	monosetTimeOut((1000-millis()%1000),tickstatic,i);
}


template <typename... ParamTypes>
void anotherThread(void (func)(ParamTypes...), ParamTypes... parames)
{

	std::thread([func,parames...]()
			{
		func(parames...);
			}).detach();
}
void parraleltask(){
	std::cout << "parraleltask... " <<count++<< std::endl;
}

void parralelAdd(){
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	std::cout << "parralelAdd setTimeout... "  << std::endl;
	monosetTimeOut(100,parraleltask);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "parralelAdd setTimeout... "  << std::endl;
	monosetTimeOut(100,parraleltask);
}
 */



#endif
