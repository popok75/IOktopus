
var jp=JSON.parse;
var js=JSON.stringify;
var geid=document.getElementById.bind(document);


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//loading doc

function initclient() {
	var model=new RemoteModel();
	var view=new UserView();
	var controller=new Controller(model,view);
	model.init();
	geid('userinterface').innerHTML="client initialized";

}





function loadJSScripts(callback) {	
	var files=[ "js/EventEmitter.js",
 		"js/RemoteModel.js",	          	// here is a copy of remote model that is synced with the remote server, modifications await server confirmation before committed
 		"js/Controller.js",					// here the controller using each node interface to match the model
 		"js/ui/dygraph.min.js",
//		"js/ui/ui-dygraph.js",
 		"js/ui/Component.js",					// here go all the html, components can be panels or other interactive items
 		"js/ui/UserView.js",					// here we have the interface for each node
 		"js/ui/interaction.js",
 		"js/ui/synchronizer.js",
 		"js/ui/LogDygraph.js"
		
		];

	loadMultiJS(files,callback);
}
var multifile=true;







//Basic window on load
window.onload = function() {

	
	 

	

	loadFav(function(){});
	loadBottomImage(function(){});
	if(multifile) loadJSScripts(initclient);	// for dev purpose each class is its own file
	else initclient();							// for production, one file is made containing all the classes

	/*loadBottomImage(function(){
	loadFav(function(){loadAllModel();});
	});*/

	//	<img src='/favicon.png' alt='IOktopus' style='width:120px;'>
	//<link rel='icon' href='/favicon.png' sizes='32x32'> 

};

function loadFav(callback) {
	var link=document.createElement('link');	
	link.onload = callback;	//doesn't work if loadFav is called immediately, works after some time

	link.rel='icon';
	link.sizes='32x32';
	link.href='/favicon.png?v1';
	document.getElementsByTagName('head')[0].appendChild(link );
}
function loadBottomImage(callback) {
	var img=document.createElement('img');
	img.src='/favicon.png' ;
	img.alt='IOktopus'; 
	img.style='width:120px;'; 
	img.onload = callback;
	document.getElementsByTagName('body')[0].appendChild(img );
}

var loadJS = function(url, callback, location){
	//url is URL of external file, implementationCode is the code
	//to be called from the file, location is the location to 
	//insert the <script> element
	if(location==null) location =document.getElementsByTagName('head')[0];
	var scriptTag = document.createElement('script');
	scriptTag.src = url;

	scriptTag.onload = callback;
	scriptTag.onreadystatechange = callback;

	location.appendChild(scriptTag);
	console.log("loading remote script "+url);
};

function loadMultiJS(files, callback){
	if(files.length==1) return loadJS(files[0],callback);
	var f2=files.slice(1,files.length);
	loadJS(files[0],function(){
		loadMultiJS(f2, callback);		
	});		
}





/////////////////////////////////
//Remote Loading not parallel loading
var callpile=[],callunpiling=false;
function loadremote(addr,cb,initf){//,ret,type,rg,xhttp){
	var xhttp = new XMLHttpRequest();
	var it=[addr,cb,initf];//ret,type,rg,xhttp];
	callpile.push(it);
	//	print("pileToFlash this.pile",pg);
	if(callpile.length==1 && !callunpiling) setTimeout(unpile,500,null);
	callunpiling=true;
	return xhttp;
};
function unpile(){
	var it=callpile.shift();
//	print("unpiled ",it);
//	plug.log.saveToFlash(it);
//	print("plug saving to flash");
	function cback(t,xhttp){
		it[1](t,xhttp);
		if(callpile.length>0) setTimeout(unpile,50,null);
		else callunpiling=false;
	}
	realloadremote(it[0],cback,it[2]);//,it[3],it[4],it[5]);

	//		 
}


//var firsttime=true;
function realloadremote(addr,cb,initf){//,retries,type,rg,xhttp){
	var  xhttp = new XMLHttpRequest();
//	var maxretry=10;
//	if(!retries) retries=0; 

	xhttp.onreadystatechange = function() {
		var len=   xhttp.responseType=="" || xhttp.responseType=="text" ? xhttp.responseText.length : xhttp.response ? xhttp.response.length :-1;
		if (this.readyState == 4 && (this.status == 200 || this.status == 206 || this.status==202)) {
			laststatus=this.status;
			console.log("connection status:"+this.status);
			var d=this.getResponseHeader("Date");
			if(d) {
				if(!stimer)	stimer=new ServerTimer(d);
				else {
					stimer.setTimeString(d);
					if(!stimer.interval) stimer.start();
				}
			}
			if(xhttp.response) cb(xhttp.response,xhttp); else cb(xhttp.responseText,xhttp);
		}else 
		{if (this.readyState == 4 && (this.status == 503)) {
			console.log("connection status:"+this.status);
			var sec=this.getResponseHeader("Retry-After");
			if(sec) setTimeout(realloadremote,sec*1000,addr,cb,retries,type,rg);
		}
		if(this.readyState == 4 && (this.status==0 || this.status==404 || this.status==304)) {
			console.log("connection status:"+this.status);
			laststatus=this.status;
			cb(null,xhttp);
		};
		};
	};
	xhttp.open("GET", addr, true);
	/*
	if(firsttime) {
		xhttp.setRequestHeader('x-Date', (new Date()).getTime());
		firsttime=false;
	}*/
//	if(rg) xhttp.setRequestHeader("Range", rg);
//	if(type) {xhttp.responseType = type;console.log("asked for binary",addr);}
//	console.log("loadremote type",addr,type);
	if(initf) initf(xhttp);
	xhttp.send();

};

