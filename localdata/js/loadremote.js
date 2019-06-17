



/////////////////////////////////
//Remote Loading not parallel loading
var callpile=[],callunpiling=false;
function loadremote(addr,cb,opt){//,ret,type,rg,xhttp){
	var xhttp = new XMLHttpRequest();
	if(typeof(opt)==="function") opt={initf:opt};
	var it=[addr,cb,opt];//ret,type,rg,xhttp];
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
	loadRemoteNow(it[0],cback,it[2]);//,it[3],it[4],it[5]);

	//		 
}


//loadRemote : load a remote addr and call cb with it's content+xhttp.  retries can be specified
function loadRemoteNow(addr,cb,opt){	// opt :{initf:function(xhttp){}, retryCallback:function(response,xhttp){}, retries:Number()}		
	if(typeof(opt)==="function") opt={initf:opt}; // can be called with initf as 3rd argument with no object
	var  xhttp = new XMLHttpRequest();
	var maxretries=10,retries;
	if(opt && opt.retries) retries=opt.retries; else retries=maxretries;		// retries may cause Delay for next script in context of sequential multiple script load -> retries function that make your own retry from response with default that retries 10 times + sequencial that just set a timer that will pile/load it later 
	xhttp.onreadystatechange = function() {
		var len= xhttp.responseType=="" || xhttp.responseType=="text" ? xhttp.responseText.length : xhttp.response ? xhttp.response.length :-1;
		if (this.readyState == 4 ){
			console.log("connection status:"+this.status);
			if (this.status == 200 || this.status == 206 || this.status==202) {
				if(xhttp.response) cb(xhttp.response,xhttp); else cb(xhttp.responseText,xhttp);
			} else if(this.status == 503) {
				function defaultRetryCallback(response,xhttp){
					var sec=xhttp.getResponseHeader("Retry-After");
					if(sec && retries>0) {setTimeout(loadRemote,sec*1000,addr,cb,opt);console.log("retry after :"+sec+"sec");}
					else cb(null,xhttp);					};
					if(opt && opt.retryCallback) opt.retryCallback(xhttp.response,xhttp);else defaultRetryCallback(xhttp.response,xhttp);
			} else if((this.status==0 || this.status==404 || this.status==304)) cb(null,xhttp);
		};
	};
	console.log("loadRemote "+addr);
	var method="GET";
	if(opt && opt.method) method=opt.method;
	xhttp.open(method, addr, true);
	if(opt && opt.initf) opt.initf(xhttp);
	if(opt && opt.tosend) xhttp.send(opt.tosend);
	else xhttp.send();
	
};
 



function loadFav(url,callback) {
	var link=document.createElement('link');	
	link.onload = callback;	//doesn't work if loadFav is called immediately, works after some time

	link.rel='icon';
	link.sizes='32x32';
	link.href=url;
	document.getElementsByTagName('head')[0].appendChild(link );
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


//export {loadremote,loadFav, loadMultiJS};
globalThis.globalexport({loadremote,loadFav, loadMultiJS});


