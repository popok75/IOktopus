

import {EventEmitter} from "./EventEmitter.js";
import {} from './loadremote.js';

var jp=JSON.parse;
var js=JSON.stringify;

var MILLIFACTOR=1000;	//ts in seconds not in millis


String.prototype.replaceAll = function(search, replacement) {
	var target = this;
	return target.split(search).join(replacement);
};

function isDigit(n){return !isNaN(parseFloat(n));}

var LOGURL="/log/";
var NEWSTREAMTAG="newstream";

class RemoteModel extends EventEmitter {

	constructor(espip0){
		super();
		if(espip0) this.espip=espip0; else this.espip="";
		this.refreshperiodsec=10;
		
		this.serverinfo={};
		console.log("RemoteModel");		
		this.rlog=new RemoteLog();
	}	

	load(){
//		this.loadEmit(this.espip+"/data","load","data","datats");
		loadremote(this.espip+"/data",function(t,xhttp){
			this.updateServerStatus(xhttp);
			var obj={};
			var b=this.parseContent(t,obj);
			if(!b) return;
			if(!obj.result || obj.result.length==0) return ;

			this.updateServerOnline("data");
			this.datats=new Date().getTime();
			this.data=obj.result;				
			this.emit("load",""); // emit
		}.bind(this),
		this.prerequest.bind(this));
	};

	reload(){
		//	this.loadEmit(this.espip+"/data","reload","data","datats");
		loadremote(this.espip+"/data",function(t,xhttp){
			this.updateServerStatus(xhttp);
			var obj={};
			var b=this.parseContent(t,obj);
			if(!b) return;
			if(!obj.result || obj.result.length==0) return ;

			this.updateServerOnline("data");
			this.datats=new Date().getTime();
			this.data=obj.result;				
			this.emit("reload",""); // emit
		}.bind(this),
		this.prerequest.bind(this));
	};


	getLastLogTS(){
		return this.log[this.log.length-1][0];
	}



	init(){
		this.load();
		this.refreshInterval=setInterval(function(){
			this.reload();
		}.bind(this), this.refreshperiodsec*1000);

		this.rlog.init(this);
		console.log("Model init ended");

	}


	updateServerStatus(xhttp){
		this.serverinfo.status="online";
		var boottime=xhttp.getResponseHeader("x-BootTime");
		if(boottime) {
			this.serverinfo.boottime=boottime;
			console.log("boottime:"+boottime);
		}
	}
	updateServerOnline(eventname){
		this.serverinfo[eventname]="ok";
		this.emit("serverinfo","");
	}	
	parseContent(t,obj){
		if(!t || t.length==0) {
			console.error("Received no content, server offline ? "+":"+t);
			this.serverinfo.status="offline";
			this.emit("serverinfo","");
			return false;
		}
		this.serverinfo.lastresponsetime=new Date().getTime();
		var tjp;
		try{
			tjp=jp(t);
		} catch(err){
			console.error("received non parsable json:"+t);//return;
			this.serverinfo.status="error";
			this.emit("serverinfo","");
			return false;
		}		
		obj.result=tjp;
		return true;
	}

	prerequest(xhttp){//if(!this.serverinfo.boottime) 
		xhttp.setRequestHeader('x-BootTime', "?");// ask each time in case of retart // should be notification
	}

	findByPath(path){
		var arr=path.split('/');
		var obj=this.data;
		for(var k in arr){
			if(!arr[k]) continue;
			if(!(arr[k] in obj)) return null;
			obj=obj[arr[k]];
		}
		return obj;
	}

	setValue(path,value){
		var leaf=path.split("/").pop();
		var obj=this.findByPath(path.substring(0,path.length-leaf.length));
		if(!obj) return;
		obj[leaf]=value;
	}
	
	userChange(path,value){
		console.log();		
		// send a request of modification to server, if the reply is ok, 
				// it is commited locally and then overwritten by the server model update
		var leaf=path.split("/").pop(), branch=path.substring(0,path.length-leaf.length);
		var url=this.espip+"/data/"+branch;
		loadremote(url,function(t,xhttp){
			console.log(t);
			// if ok notify back 
			if(t=="OK"){
				this.setValue(path,value);
				this.emit("confirmChange",path);				
			} else if(t=="NOTOK") {
				this.emit("cancelChange",path);
			}
		}.bind(this),
		{method:"PUT",tosend:leaf+"="+value});
	
	};


	updateServerError(){
		this.serverinfo.status="error";
		this.emit("serverinfo","");
	}
}



////////////////////////////////////////////////////////////////////////
class RemoteLog {
	//////////////////////////////
	constructor(){
		this.logrefreshperiodsec=10;
	}
	//////////////////////////////
	init(rmodel){
		this.loadlog(rmodel);
		this.refreshInterval=setInterval(function(){
			this.reloadlog(rmodel);
		}.bind(this), this.logrefreshperiodsec*1000);

	}
	
	
	//////////////////////////////
	loadlog(o){
		this.o=o;
		//	this.loadEmit(this.espip+"/log","logload","log","logts");
		loadremote(o.espip+LOGURL,function(t,xhttp){
			this.updateServerStatus(xhttp);
			var obj={};
			var b=this.parseContent(t,obj);
			if(!b) return;
			if(!obj.result || obj.result.length==0) return ;

			this.updateServerOnline("log");
			b=this.rlog.processLog(obj.result);		
			delete obj.result;
			if(b) {
				this.series=getSeriesLogv15(this.log, Object.keys( this.rlog.lognames));
				this.labels=['Date'].concat(Object.keys( this.rlog.lognames));
				this.emit("logload",""); // emit
			}
		}.bind(o),
		o.prerequest.bind(o));
	};

	//////////////////////////////
	reloadlog(o){
		if(this.downloading) return;
		if(this.logversion=="0.16"){
			this.o=o;
			//	this.loadEmit(this.espip+"/log","logload","log","logts");
			var xtra=this.log[this.log.length-1][0];
			if(isDigit(xtra)) xtra="?fromTS="+parseInt(xtra);
			else xtra="";
			loadremote(o.espip+LOGURL+xtra,function(t,xhttp){			// add last ts here 
				this.updateServerStatus(xhttp);
				var obj={};
				var b=this.parseContent(t,obj);
				if(!b) return;
				if(!obj.result || obj.result.length==0) return ;

				this.updateServerOnline("log");
				b=this.rlog.processLog(obj.result);						
				if(b) {
					//this.series=getSeriesv1(rawdata,seriesnamesy.concat(seriesnamesy2));
					this.series=getSeriesLogv15(this.log, Object.keys( this.rlog.lognames));
					this.labels=['Date'].concat(Object.keys( this.rlog.lognames));
					this.emit("logreload",""); // emit
				}
			}.bind(o),
			o.prerequest.bind(o));
			
		} else if(this.logversion=="0.20"){
			this.reloadLogv020(o);	
		}	
	};
	
	 
	//////////////////////////////
	parseID(filename){
		var num="";
		for(var c in filename){
			if(isDigit(filename[c])) num+=filename[c];
		}
		return parseInt(num);
	}
	//////////////////////////////
	processLog(data){
		if(typeof(data[0])=="string") {	//version 0.2
			this.logversion="0.20";
			this.filenames=data;
			var b=this.loadLogFiles(data);	//files are sequencially loaded, merged and displayed
			return b;
		} else {	// version 0.16
			this.logversion="0.16";
			var b= this.mergeLog(data);	
			this.o.log=this.log;
			return b;
		}

	};
	
	//////////////////////////////
	extract(t){	// a better way do the same is to extract each separately (and check the tag newstream, column names, or sync) and add them
		var full =this.lastlogfileraw+t;	//keep the header and last data only  & store size elsewhere
		var raw="";
		var start=full.lastIndexOf("[["),start2=full.lastIndexOf("[{");
		if(start2>start) start=start2;
		var jindex=full.indexOf("{",start);
		var index=full.lastIndexOf("}");
		//index=full.indexOf("}",index+1);
		if(index>0) raw="["+full.substr(jindex,index+1-jindex)+',';
		if(t[0]!=']') raw+=full.substr(full.lastIndexOf("["));// get raw from last ts only
	
		return raw;
	}
	//////////////////////////////
	loadLogFiles(filenames,min=0){
		this.downloading=true;
		var index=filenames.length-1;
		var recfunc=function(t,xhttp){
			if(t){
				console.log("t:"+t);
				if(t.substring(t.length-2,t.length)!="]]") {//file incomplete still under update
					this.lastlogfileincomplete=true;
					this.lastlogfilesize=t.length;
					this.lastlogfileraw="";
					this.lastlogfileraw=this.extract(t);		// remember just the header and the last data line
					console.log("extract : "+this.lastlogfileraw);
					t+="]]";
				}
				t="["+t+"]";
				var dataobj;
				try{
					dataobj=JSON.parse(t);	
				}catch(err){
					console.error("received non parsable json log file "+filenames[this.index]+":"+t);//return;
					this.o.updateServerError();
					return false;
				}
				for(var k in dataobj) this.mergeLog(dataobj[k]);
				dataobj=[];	// free some memory

				this.o.log=this.log;
				this.o.series=getSeriesLogv15(this.log, Object.keys( this.lognames));
				this.labels=['Date'].concat(Object.keys( this.rlog.lognames));
				this.o.emit("logload",""); // emit				
			}
			if(recfunc.index>min) {
				recfunc.index--;
				loadremote(this.o.espip+LOGURL+filenames[recfunc.index],recfunc.bind(this));
			} else this.downloading=false;
			return true;
		};
		recfunc.index=index;
		loadremote(this.o.espip+LOGURL+filenames[recfunc.index],recfunc.bind(this));
		return false;
	};
	
	
	
	
	

	////////////////
	reloadLogv020(o)	{
		if(this.lastlogfileincomplete){// if was open, reload last file in the list from position size, concat with previous copy
			this.downloading=true;
			loadremote(o.espip+LOGURL+this.filenames[this.filenames.length-1]+"?range="+this.lastlogfilesize+"-",function(t,xhttp){
				this.o.updateServerStatus(xhttp);
				if(t){
					if(t=="]]") {this.lastlogfileincomplete=false;this.reloadLogv020(this.o);return true;}//no data, just end of file, load next
					var tinit=t;	// for debug
					var prevrawinit=this.lastlogfileraw; // for debug
					this.lastlogfilesize+=t.length;	//keep the header and last data only  & store size elsewhere
					 
					if(t.substr(0,2)==']]'){	// start with end of dataline and end of stream
						this.lastlogfileraw="";// remove end
						t=t.substr(3);		
					} else if(t[0]==']') { // start with a end of dataline only				
						this.lastlogfileraw=this.lastlogfileraw.substr(0,this.lastlogfileraw.lastIndexOf("["));// remove end
						t=t.substr(2);
					}
						
					if(t.substring(t.length-2,t.length)!="]]") this.lastlogfileincomplete=true;// end with end of stream
					else this.lastlogfileincomplete=false;
				
					var sum=this.lastlogfileraw+t;
					if(this.lastlogfileincomplete) {this.lastlogfileraw=this.extract(sum);sum+="]]";}
					else this.lastlogfileraw="";

					t="["+sum+"]";
						
					var dataobj;
					try{
						dataobj=JSON.parse(t);	
					}catch(err){
						console.error("received non parsable json log file "+this.filenames[this.index]+":"+t);//return;
						this.o.updateServerError();
						return false;
					}
					for(var k in dataobj) this.mergeLog(dataobj[k]);	// merge same ts
					this.o.log=this.log;
					this.o.series=getSeriesLogv15(this.log, Object.keys( this.lognames));
					this.labels=['Date'].concat(Object.keys( this.rlog.lognames));
					this.o.emit("logreload",""); // emit				
				}
				this.downloading=false;
				return true;
			}.bind(this),
			o.prerequest.bind(o));
		} else{// if found closed, reload log file list and load the last of the list
			this.o=o;
			var id=this.parseID(this.filenames[this.filenames.length-1])+1;
			loadremote(o.espip+LOGURL+"?fromID="+id,function(t,xhttp){	//add last id here
				this.o.updateServerStatus(xhttp);
				var obj={};
				var b=this.o.parseContent(t,obj);
				if(!b) return;
				if(!obj.result || obj.result.length==0) return ;

				this.o.updateServerOnline("log");
				if(obj.result) {
					var sz=this.filenames.length;
					for(var k in obj.result) this.filenames.push(obj.result[k]); 
					b=this.loadLogFiles(this.filenames,sz);
					if(b) this.emit("logload",""); // emit
				}
				
				//b=this.rlog.processLog(obj.result);
			}.bind(this),
			o.prerequest.bind(o));
		}
		
	};
	
	
	
	//////////////////////////////
	mergeLog(data){
		if(!data || data.length==0 || data[0].length==0) {console.log("Log data table received from /log/ empty.");return false;}
		this.logts=new Date().getTime();
		this.prepareData(data);
		if(!this.log) this.log=[];//ata;return true;}

		//merge names firs
		//	if(isNaN(parseInt(data[0][0]))){
		//		this.log[0]=[...new Set([...this.log[0], ...data[0][0]])];
		//	}
		if(!this.lognames) this.lognames={};//=data.shift();
		for(var j in data){
			var b1=!Array.isArray(data[j]);
			var b2=!data[j].sync;
			if(!Array.isArray(data[j]) && !data[j].sync){// an object not array that has no sync property
				Object.assign(this.lognames, data[j]);//this.lognames=[...new Set([...this.lognames, ...data[j]])]; 
				break;
			}
		}
		this.log[0]=this.lognames;
		// we keep a simplified log model, with log[0] column names, [newstream],+normal entries with absolute ts

		// each dataline is inserted where it belongs temporally
		// newstream is inserted with the next dataline
		var nextns=false;
		var startline=0;
		for (var i in this.log) {
			if(isNaN(parseInt(this.log[i][0]))) continue;
			startline=i;
			break;
		}
		console.log("mergeLog : "+data[1]);
		for (var i in data) {
			if(data[i][0]==NEWSTREAMTAG) nextns=true;
			if(isNaN(parseInt(data[i][0]))) continue;

			// if after end push back, if before start push front, if in between find where
			if(this.log.length==1) {if(nextns) this.log.push([NEWSTREAMTAG]);this.log.push(data[i]);startline=1;}
			else if(data[i][0]>this.log[this.log.length-1][0]) {if(nextns) this.log.push([NEWSTREAMTAG]);this.log.push(data[i]);}
			else if(data[i][0]<this.log[startline][0]) {if(nextns) this.log.splice(startline,0,[NEWSTREAMTAG]); this.log.splice(startline,0,data[i]);}
			else { 
				// 0 is names, 1 is ?
				for(var k=startline;k<this.log.length;k++) {
					if(this.log[k][0]==data[i][0]) {this.mergeLine(data[i],this.log[k]);break;}// do nothing if we already have this data line
					if(this.log[k][0]>data[i][0]) {
						if(k>0 && isNaN(parseInt(this.log[k-1][0]))) k--;
						this.log.splice(k,0,data[i]);	
						if(nextns) {this.log.splice(k,0,[NEWSTREAMTAG]);}
						break;
					}
				}
			}
			nextns=false;
		}
		return true;    
	};

	
	//////////////////////////////
	mergeLine(srcline, dstline){
		for(var i=1;i<srcline.length;i+=2){
			var found=false;
			for(var j=1;j<dstline.length;j+=2){
				if(srcline[i]==dstline[j]) {// if we already have this column, 
					if(srcline[i+1]!=dstline[j+1]) dstline[j+1]=srcline[i+1];	//	different value update it
					found=true;
					break;	// with same value skip it
				}	
			}
			if(!found) {dstline[dstline.length]=srcline[i];dstline[dstline.length]=srcline[i+1];}
		}
	}
	
	
	//////////////////////////////
	prepareData(data){
		var ts0=0;
		for(var i in data){
			//if(data[i[0]]=="newstream") data.splice(i,1);
			if(data[i].sync && !isNaN(parseInt(data[i].sync))) {ts0=data[i].sync;/*data.splice(i,1);*/continue;}
			else if(!Array.isArray(data[i]) || isNaN(parseInt(data[i][0]))) continue;
			data[i][0]=ts0+data[i][0];
		}
	};
	
	
	/*
	prepareData(rawdata){
		var seriesnamesy=getPropertySubArray(this.config.series.y,"name");
		var seriesnamesy2=getPropertySubArray(this.config.series.y2,"name");
		if(!Array.isArray(rawdata)){
			var series=getSeriesv1(rawdata,seriesnamesy.concat(seriesnamesy2));
			return series;			
		} else {
			var series=getSeriesv15(rawdata,seriesnamesy.concat(seriesnamesy2));
			return series;
		}

		return data;
	}*/

};

//////////////////////////////////////
function getSeriesLogv15(data0,names){
	var series=[];
	var i=1;
	var size=Object.keys(names).length+1;

	var data=data0.slice(0);
	var fnames={};//=data.shift();
	for(var j in data){
		var b1=!Array.isArray(data[j]);
		var b2=!data[j].sync;
		if(!Array.isArray(data[j]) && !data[j].sync){// an object not array that has no sync property
			fnames=data[j];
			break;
		}
	}
	
	var gfd={};
	for(var n in names){
		for(var m in fnames){
			if(m==names[n]) {gfd[fnames[m]]=Number.parseInt(n); break;} 
		}
	}
//	var firstts=0;
	var sz=names.length;
 //	var names2=names.splice(0);
//	names2.splice(0,0,"Date");
//	series.push(names2);
	for (var dl in data){
		var l=data[dl];
		if(!Array.isArray(l)) continue;
		if(isNaN(parseInt(l[0]))) continue;
	//	if(!firstts) firstts=l[0];
	//	else l[0]+=firstts;
		var ms=l[0]*MILLIFACTOR;
		var gl=Array(sz+1);
		gl.fill(null);
		gl[0]=new Date(ms);
		for(var i=1;i<l.length;i+=2){
			var nc=gfd[l[i]]+1;
			var nv=Number.parseFloat(l[i+1]);
			gl[nc]=nv;
		}
		series.push(gl);
	}
	return series;
}

export {RemoteModel};