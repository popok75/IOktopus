#ifndef PSYCHRODRIVER_H
#define PSYCHRODRIVER_H

#include "SensorDriver.h"
#include <Math.h>

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.psychrodriver"

class IOFactory;
SensorDriver *IOFactorycreateDriver (GenString model0, unsigned int num, std::vector<unsigned int> &pins);	// definition in IOFactory.h

float getPsychroEstimate(float wet, float dry){
//	float hum;
// https://thecavepearlproject.org/2015/07/08/measuring-humidity-in-a-cave-a-masons-hyrgometer-experiment/
	/*
Saturation Vapor Pressure @ wet bulb temp:
	= 0.61078*EXP((17.08085*T(wet))/(237.175+T(wet)))
Actual Vapor Pressure:
	= Sat. V.P.@wet bulb – [ (psy. constant) * (Atm.Pressure in kPa) * (T(dry)-T(wet)) ]
Relative Humidity:
	= (Actual V.P./ Saturation V.P.)*100
*/	if (wet>dry) {float t=wet;wet=dry;dry=t;}
/*
	double psyconst=0.002, pressure=101.325;
	double svp=0.61078*exp((17.08085*wet)/(237.175+wet));//	double svp=0.61078*pow(10,(17.08085*wet)/(237.175+wet));
	double avp=svp-psyconst*pressure*(dry-wet);
	double rh=100*avp/svp;
*/
// https://www.1728.org/relhum.htm
	//ew=svp, ed is new
	double correction=2.3;
	double N=.6687451584*correction;
	double ed=6.112*exp(17.502*dry/(240.97+dry));
	double ew=6.112*exp(17.502*wet/(240.97+wet));
	double rh=100*(ew-N*(1+.00115*wet)*(dry-wet))/ed;
//	rh=100-(100-rh)*correction;


//http://www.bom.gov.au/climate/averages/climatology/relhum/calc-rh.pdf
 /*   double psyconst=0.0007866, pressure=1013.25;
    double svp=exp(1.8096+(17.2694*wet)/(237.3+wet));//	double svp=0.61078*pow(10,(17.08085*wet)/(237.175+wet));
	double avp=svp-psyconst*pressure*(dry-wet)*(1+wet/610);
	double rh=100*avp/svp;
*/
	println(GenString()+RF("psychro dry:")+ to_string(dry)+RF(" wet:")+to_string(wet) +RF(" rh:")+to_string(rh));
	return rh;
}

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.psychroreader2"

class PsychroDriver : public SensorDriver
{
	SensorDriver *wetbulb,*drybulb=0;//= Sensirion(dataPin, clockPin);
	//FakeHTU htu = FakeHTU();	// could use a real htu too
	float wetval=0, dryval=0, humval=0;
	bool waitingwet=true, waitingdry=true;
	bool drydisconnected=false, wetdisconnected=false;
	unsigned int pinwet=0,pindry=0;

public:
	PsychroDriver(unsigned int num, std::vector<unsigned int> pins):SensorDriver(num,pins){
/*
		wetname=RF("Temperature-Wet");
		if(!wetname0.empty()) wetname=wetname0;
		dryname=RF("Temperature-Dry");
		if(!dryname0.empty()) dryname=dryname0;

		humname=RF("Humidity");
		if(!humname0.empty()) humname=humname0;
*/
		if(pins.size()==2) pinwet=pins[0],pindry=pins[1];

	}
	~PsychroDriver(){if(wetbulb) delete wetbulb;if(drybulb) delete drybulb;}

	virtual unsigned int channelNumber(){return 3;};
	virtual GenString channelName(unsigned int i){
		if(i==0) return GenString()+RF(TEMPERATURE_CHANNEL_TYPE)+RF("-DryBulb");
		if(i==1) return GenString()+RF(TEMPERATURE_CHANNEL_TYPE)+RF("-WetBulb");
		if(i==2) return RF(HUMIDITY_CHANNEL_TYPE);
		return "";
	};
	virtual GenString channelType(unsigned int i){
		if(i==0) return RF(TEMPERATURE_CHANNEL_TYPE);
		if(i==1) return RF(TEMPERATURE_CHANNEL_TYPE);
		if(i==2) return RF(HUMIDITY_CHANNEL_TYPE);
		return "";
	};
	virtual bool isConnected(unsigned int i){
		if(i==0) return !drydisconnected;
		if(i==1) return !wetdisconnected;
		if(i==2) return (!drydisconnected || !wetdisconnected);
		return false;
	}
	virtual float value(unsigned int i){
		if(i==0) return dryval;
		if(i==1) return wetval;
		if(i==2) return humval;
		return NAN;
	};

	bool init() {

		//	if (!htu.begin()) {println("Couldn't find sensor HTU!");return false;}
 		println(RF("PsychroReader::init on pins:")+to_string(pinwet)+RF(", ")+to_string(pindry));

 		std::vector<unsigned int > pinswet = {pinwet};
		wetbulb=IOFactorycreateDriver(RF("DS18B20"), 100+num,pinswet);
		if(!wetbulb) return false;
		wetbulb->init();	// skip init message from sensor, replaced below


		std::vector<unsigned int > pinsdry = {pindry};
		drybulb=IOFactorycreateDriver(RF("DS18B20"), 100+num,pinsdry);
		if(!drybulb) return false;
		drybulb->init();

 		println(RF("PsychroReader init with success!"));

/*
		//could emit one event only
		StringMapEvent arg({{humname+RF("/type"),RF("input")},{humname+RF("/unit"),RF("%")}});//		std::multimap<std::string,std::string>valmap={{"Humidity/type","input"},{"Humidity/unit","%"}};//,{"ts",to_string(CLOCK.getTimeMS())}};
		emit(&arg);

		StringMapEvent arg2({{wetname+RF("/type"),RF("input")},{wetname+RF("/unit"),RF("°C")}});//		valmap={{"Temperature/type","input"},{"Temperature/unit","°C"}};//,{"ts",to_string(CLOCK.getTimeMS())}};
		emit(&arg2);

		StringMapEvent arg3({{dryname+RF("/type"),RF("input")},{dryname+RF("/unit"),RF("°C")}});//		valmap={{"Temperature/type","input"},{"Temperature/unit","°C"}};//,{"ts",to_string(CLOCK.getTimeMS())}};
		emit(&arg3);
*/
		return true;
	};

/*
	bool notify(GenString ename,Event*event=0){
		// if one sensor disconnected -> hum=disconnected and other continue update

	//	println(GenString()+"PsychroReader::notify ");
		StringMapEvent *emap=0;
		if(event->getClassType()==StringMapEventTYPE) emap=(StringMapEvent*)(event);
		if(!emap || emap->values.empty()) return false; //pb
		println(GenString()+"PsychroReader::notify "+emap->values.asJson());

		// save the value, wait for the other sensor
		std::string str=emap->values.get(dryname+RF("/val"));
		bool dry=true;//, disconnected=true;

		if(str.empty()){
			str=emap->values.get(wetname+RF("/val"));
			waitingwet=false;
			if(str==RF(DISCONNECTED_STATE) || str==RF(INIT_STATE) || str.empty()) {
				wetdisconnected=true;
				saveValue(wetname, str);
				if(!waitingdry) saveValue(humname, RF(DISCONNECTED_STATE));
				return true;
			}
			wetdisconnected=false;
			dry=false;
		} else {
			waitingdry=false;
			if(str==RF(DISCONNECTED_STATE) || str==RF(INIT_STATE)) {
				drydisconnected=true;
				saveValue(dryname, str);
				if(!waitingwet) saveValue(humname, RF(DISCONNECTED_STATE));
				return true;
			}
			drydisconnected=false;
		}
		float f=strToDouble(str);
		if(dry) dryval=f; else wetval=f;

		if(!waitingdry && !waitingwet) {
			if(drydisconnected || wetdisconnected) {// if one sensor fail, the other will update its value and humidity is disconnected
				if(!wetdisconnected) {saveValue(wetname, to_stringWithPrecision(wetval,2));}
				if(!drydisconnected) {saveValue(dryname, to_stringWithPrecision(dryval,2));}
				saveValue(humname, RF(DISCONNECTED_STATE));
			} else {// if we have 2 values, we update the humidity
				// dryval is always bigger than wetval
				if(dryval<wetval){float t=dryval;dryval=wetval;wetval=t;wetbulb->rename(dryname);drybulb->rename(wetname);} // maybe reader names should be switched ?
				saveValue(dryname, to_stringWithPrecision(dryval,2));
				saveValue(wetname, to_stringWithPrecision(wetval,2));
				humval=getPsychroEstimate(dryval,wetval);
		 		saveValue(humname, to_stringWithPrecision(humval,2));
			}
		}

		return true;
	}



	bool read(){
 		if(!reading) {println("Psychro::read !reading");
			reqms=millis();
			wetbulb->read();
			drybulb->read();
			reading=true;
			waitingwet=true,waitingdry=true;
			return true;
		} else {println("Psychro::read reading");
			return false;
		}
 	}
*/
 	bool sensorTick(){
 		//bool ret=false;
 		if(waitingwet && wetbulb->sensorTick()){
 			waitingwet=false;
 			if(wetbulb->isConnected(0)) {
 				wetval=wetbulb->value(0);
 			//	println(RF("Wet bulb updated with :")+to_stringWithPrecision(wetval,2));
 				wetdisconnected=false;}
 			else wetdisconnected=true;
 			if(drydisconnected) {waitingdry=true;waitingwet=true;return true;}
 		};
 		if(waitingdry && drybulb->sensorTick()){
 			waitingdry=false;
 			if(drybulb->isConnected(0)) {
 				dryval=drybulb->value(0);
 			//	println(RF("dry bulb updated with :")+to_stringWithPrecision(dryval,2));
 				drydisconnected=false;}
 			else drydisconnected=true;
 			if(wetdisconnected) {waitingdry=true;waitingwet=true;return true;}
 		};

 		if(!waitingdry && !waitingwet) {
			if(!drydisconnected && !wetdisconnected) {
				if(dryval<wetval){
					float t=dryval;dryval=wetval;wetval=t;
					SensorDriver *tmp=drybulb;drybulb=wetbulb;wetbulb=tmp;
				}
				humval=getPsychroEstimate(dryval,wetval);
			}
			waitingdry=true;waitingwet=true;
			return true;
		}

 		return false;
 	} // do nothing & stop the ticker

};

#endif

