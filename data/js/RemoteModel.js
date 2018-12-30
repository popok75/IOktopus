
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
	
	loadEmit(url,eventname,obj,objts){
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
			this[objts]=new Date().getTime();
//			console.log("received "+eventname+":"+t);
			this[obj]=tjp;
			//debug display
//			geid('userinterface').innerHTML=t;
			//console.log(eventname+":"+t);
			//emit
			this.emit(eventname,"");
		}.bind(this),
		function(xhttp){
			//if(!this.serverinfo.boottime) 
				xhttp.setRequestHeader('x-BootTime', "?");// ask each time in case of retart // should be notification
			}.bind(this));
	};
	
	load(){
		this.loadEmit(this.espip+"/data","load","data","datats");
	};
	
	reload(){
		this.loadEmit(this.espip+"/data","reload","data","datats");
	};
	
	loadlog(){
		this.loadEmit(this.espip+"/log","logload","log","logts");
	};
	
	reloadlog(){
		this.loadEmit(this.espip+"/log","logreload","log","logts");
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