

 
  
if(typeof(exportm)!=="undefined") exportm("bundler");	// register to require.js if loaded
console.log("Bundler module initializing");


//loadReamote


function loadRemote(addr,cb,opt){	// opt :{initf:function(xhttp){}, retryCallback:function(response,xhttp){}, retries:Number(), maxretries:Number()}	
	var  xhttp = new XMLHttpRequest();
	var maxretries=10,retries;
	if(opt && opt.maxretries) maxretries=opt.maxretries;
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
	xhttp.open("GET", addr, true);
	if(opt && opt.initf) opt.initf(xhttp);
	xhttp.send();
};


function seqLoadRemote(url,callback,opt){
	if(typeof(remotefileswaiting)==="undefined") remotefileswaiting=[], remotebusy=false;//initialize undeclared globals	
	if(typeof(opt)==="undefined") opt={};
	if(!("retryCallback" in opt)) 
		opt.retryCallback=function(response,xhttp){
			if(typeof(opt.retries)==="undefined") opt.retries=10;
			if(xhttp.getResponseHeader("Retry-After") && opt.retries>0) {setTimeout(seqLoadRemote,sec*1000,url,callback,opt);console.log("seqLoadRemote retry after :"+sec+"sec");
			}
	};
	if(remotebusy) {remotefileswaiting.push([url,callback,opt]);return;}
	var func=function(t,xhttp){//console.log("dowloaded script");
		remotebusy=false;
		if(remotefileswaiting.length>0) {			//var fr=scriptswaiting.front();
			var tab=remotefileswaiting.splice(0,1)[0];
			remotebusy=true;//	console.log("requesting script "+fr);
			loadRemote(tab[0],func.bind(tab[1]),tab[2]);}
		if(this) this(t,xhttp);
	};
	remotebusy=true;//	console.log("requesting script "+url);

	loadRemote(url,func.bind(callback),opt);	
}


function downloadFile(filename, data) {
	var blob = new Blob([data], {type: 'text/csv'});
	if(window.navigator.msSaveOrOpenBlob) {
		window.navigator.msSaveBlob(blob, filename);
	}
	else{
		var elem = window.document.createElement('a');
		elem.href = window.URL.createObjectURL(blob);
		elem.download = filename;        
		document.body.appendChild(elem);
		elem.click();        
		document.body.removeChild(elem);
	}
}

////////////////////////////////////////////////////////////
// Bundler
function makeBundle(cb,url){	
	var method="parse";// "preloaded";//
	if(!url) url=document.scripts[0].src;
	
	var pathmap={},pathorder=[];
	var scriptcount=document.scripts.length,requested=0;
	
	function makeBundle(map,order,cbf,foundrequire){
		var bundled="";
		for(var l in order) {// we use the right order of the loaded scripts
			var modulename=moduleName(order[l]);
			if(modulename=="bundler" || modulename=="require") continue;				
			var comment="///////////////////////////////////////////////////\n// Module : \t"+
							modulePath(order[l])+modulename+
							"\t //\n///////////////////////////////////////////////////\n";
			if(bundled) comment="\n\n"+comment;
			bundled+=comment+pathmap[order[l]];
		}
 		var found={found:false};
		bundled=removeLoadJS("require",bundled,found);
		while(found.found) {foundrequire=true;bundled=removeLoadJS("require",bundled,found);}
		bundled=removeLoadJS("bundler",bundled,found);
		while(found.found) bundled=removeLoadJS("bundler",bundled,found);
		var header="var BUNDLE=true;// added by the bundler\n";		
		if(foundrequire) header+="function require(){};\nfunction define(){};// added by the bundler\n"
		bundled=header+bundled;
		if(cbf) cbf(bundled);
	}
	
	if(method=="parse"){
		function insertPathBefore(newpath,path){
			var j=pathorder.indexOf(newpath);
			for(var i=0;i<pathorder.length;i++)	
				if(path==pathorder[i]) {
					if(j>-1 && j<i) return;
					if(j>i) pathorder.splice(j,1);
					j=i; 
					pathorder.splice(j,0,newpath);
					return;}
		};
		function getAbsUrl(relativepath,basepath){
			if(relativepath.indexOf("//")>-1) return relativepath;	//is already absolute
			var l=basepath.lastIndexOf("/");
			return basepath.substr(0,l+1)+relativepath;
		};
		function parseBundle(t,xhttp){
			pathmap[xhttp.responseURL]=t;
			var t2=removeComments(t);
			var i=0;
			while(i>=0){
				var obj=parseRequire(t2,i);
				i=obj.index;
				if(i<0) continue;
				var absurl=getAbsUrl(obj.path,xhttp.responseURL);			
				if(pathorder.indexOf(absurl)==-1) {// check that it is not already loaded
					seqLoadRemote(obj.path, parseBundle);requested++;	// at this point, the download will be async
					found=true;
				}
				insertPathBefore(absurl,xhttp.responseURL);
			}
			requested--;
			if(requested==0){
				makeBundle(pathmap,pathorder,cb);
			}
		};
//		insertPathBefore(url);
		url=getAbsUrl(url,document.scripts[0].src); //extract path from current script
		pathorder.push(url);
		seqLoadRemote(url,parseBundle);requested++;
		
	}
	
	if(method=="preloaded"){	// in this case, url is ignored
		var fulltext="";	// current document include all required scripts but as dom...
		var userequire=false;
		var bundleModule=function(t,xhttp){
			pathmap[xhttp.responseURL]=t;

			if(Object.keys(pathmap).length==scriptcount) {	
				for(var l=document.scripts.length;l>0;l--) pathorder.push(document.scripts[l-1].src);
				makeBundle(pathmap,pathorder,cb);
			}
		};

		for(var l=scriptcount;l>0;l--){ 
			if(moduleName(document.scripts[l-1].src)=="bundler") {scriptcount--;continue;}
			if(moduleName(document.scripts[l-1].src)=="require") {foundrequire=true;scriptcount--;continue;}
			seqLoadRemote(document.scripts[l-1].src,bundleModule);
		}		
	}

}


function removeLoadJS(file,t,f){
	var keyword="loadJS(\""+file
	var i=t.indexOf(keyword);
	while(i>2 && t.substr(i-2,2)=="//") i=t.indexOf(keyword,i+1);
	if(i<0) {if(f) f.found=false;return t;}
	
	
	var j=t.indexOf(");",i);
	if(j<0) {if(f) f.found=false;return t;}
	if(f) f.found=true;
	var line=t.substr(i,j+2-i),l=line.indexOf(",");
	if(l>-1) {
		var e=line.indexOf(");",l);
		line=line.substr(l+1,e-l-1)+"();\t\t// added by the bundler\n";
	} else line="";
	//if include callback replace by callback
	t=t.substr(0,i)+"//"+t.substr(i,j+2-i) +"\t\t// commented by the bundler\n"+line+t.substr(j+2);
	
	return t;
}


function parseRequire(ctext,i){
	 var keyword="require(\"";
	 var j=ctext.indexOf(keyword,i);
	 if(j<0) return {index:-1};
	 j=j+keyword.length-1;
	 var k=ctext.indexOf(");",j);	// ); at end if require is needed to parse correctly, without counting parenthesis
	 
	 var line=ctext.substr(j,k-j),l=line.indexOf(",");
	 if(l>-1) line=line.substr(0,l);
	 console.log("evaluating :"+line);
	 var ppath=eval(line);
	 
	 ppath+=".js";
	 return {index:k+2,path:ppath};
	}



function removeComments(t){
	var t2=t;
	var i=0;
	while( i> -1)
	{
		i=t.indexOf("//",i);
		if(i>-1) {
			var j=t.indexOf("\n",i+1);
			if(j>-1) t=t.substring(0,i)+t.substring(j+1);
		}
	}
	return t;
}

/*
function makeBundled(cb){
	 var fulltext="",url=document.scripts[0].src;	// current document include all required scripts but as dom...
	 realloadremote(url, function(t,xhttp){
		fulltext=t;
		 //replace each require by it's content recursively
		var pathmap={},order=[];
		var parseandload=function(t,xhttp){
			pathmap[xhttp.responseURL]=t;
			var t2=removeComments(t);
			var i=0,found=false;
			while(i>=0){
					var obj=parseRequire(t2,i);
					i=obj.index;
					if(i>=0 && !(obj.path in pathmap)) {// check that it is not already loaded

						realloadremote(obj.path, parseandload);	// at this point, the download will be async
						found=true;
					}
			}
			if(!found) {
				if(cb && Object.keys(pathmap).length==document.scripts.length) {				
					var bundled="";
					for(var l=document.scripts.length;l>0;l--) {// we use the right order of the loaded scripts
						var comment="///////////////////////////////////////////////////\n// Module : \t"+moduleName(document.scripts[l-1].src)+"\t //\n///////////////////////////////////////////////////\n";
						if(bundled) comment="\n\n"+comment;
						bundled+=comment+pathmap[document.scripts[l-1].src];
					}
					cb(bundled);
				}
			}
		}	 

		parseandload(fulltext,{responseURL:url});
	 });
 }


 


 */



