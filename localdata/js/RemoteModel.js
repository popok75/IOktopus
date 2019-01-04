
String.prototype.replaceAll = function(search, replacement) {
    var target = this;
    return target.split(search).join(replacement);
};

class RemoteModel extends EventEmitter {

	constructor(espip0){
		super();
		if(espip0) this.espip=espip0; else this.espip="";
		this.refreshperiodsec=10;
		this.logrefreshperiodsec=10;
		this.serverinfo={};
		console.log("RemoteModel");		
	}	
	
	loadEmit(url,eventname,obj,objts,ffin){
		loadremote(url,function(t,xhttp){
			this.serverinfo.status="online";
			var boottime=xhttp.getResponseHeader("x-BootTime");
			if(boottime) {
				this.serverinfo.boottime=boottime;
				console.log("boottime:"+boottime);
				}
			if(!t || t.length==0) {
				console.error("Received no content, server offline ? "+eventname+":"+t);
				this.serverinfo.status="offline";
				this.emit("serverinfo","");
				return;}
			this.serverinfo.lastresponsetime=new Date().getTime();
			var tjp;
			try{
				tjp=jp(t);
			} catch(err){
				console.error("received non json parsable "+eventname+":"+t);//return;
				this.serverinfo.status="error";
				this.emit("serverinfo","");
				return;
			}
			this.serverinfo[eventname]="ok";
			this.emit("serverinfo","");
			var b=true;
			if(obj=="log"){
				b=this.mergeLog(tjp);
			} else{
				this[objts]=new Date().getTime();
				this[obj]=tjp;				
			}
//			console.log("received "+eventname+":"+t);
			//debug display
//			geid('userinterface').innerHTML=t;
			//console.log(eventname+":"+t);
			//emit
			if(b) this.emit(eventname,"");
		}.bind(this),
		function(xhttp){
			//if(!this.serverinfo.boottime) 
				xhttp.setRequestHeader('x-BootTime', "?");// ask each time in case of retart // should be notification
			}.bind(this));
	};
	
	mergeLog(data){
		if(!data || data.length==0 || data[0].length==0) {console.log("Log data table received from /log/ empty.");return false;}
		this.logts=new Date().getTime();
		this.prepareData(data);
		if(!this.log) {this.log=data;return true;}

		//merge names firs
		if(isNaN(parseInt(data[0][0]))){
			this.log[0]=[...new Set([...this.log[0], ...data[0][0]])];
		}
		
		for (var i in data) {
			if(isNaN(parseInt(data[i][0]))) continue;
			if(data[i][0]<this.log[this.log.length-1]) continue;
			this.log.push(data[i]);
		}
		return true;    
	};
	
	prepareData(data){
		var ts0=0;
		for(var i in data){
			if(isNaN(parseInt(data[i][0]))) continue;
			if(ts0==0) {ts0=data[i][0];continue;}
			data[i][0]=ts0+data[i][0];
		}
	}
	
	load(){
		this.loadEmit(this.espip+"/data","load","data","datats");
	};
	
	reload(){
		
		this.loadEmit(this.espip+"/data","reload","data","datats");
	};
	
	loadlog(){
		this.loadEmit(this.espip+"/log","logload","log","logts");
	};
	getLastLogTS(){
		return this.log[this.log.length-1][0];
	}
	reloadlog(){
		var url=this.espip+"/log";
		if(this.log && this.log.length) url+="?from="+this.getLastLogTS();
		this.loadEmit(url,"logreload","log","logts");
	};
	 
	
	init(){
		this.load();
		this.refreshInterval=setInterval(function(){
			this.reload();
		}.bind(this), this.refreshperiodsec*1000);

		this.loadlog();
		this.refreshInterval=setInterval(function(){
			this.reloadlog();
		}.bind(this), this.logrefreshperiodsec*1000);

		console.log("Model init ended");
		
	}
 
};