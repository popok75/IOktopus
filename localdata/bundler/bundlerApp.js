
loadJS("bundler.js");


window.onload=function(){	// onload will wait we finish loading modules

	console.log("onload");


	var func=function() {		
		var filename=document.getElementById("bundlefilename").value;		 
		var scripturl=document.getElementById("mainscripturl").value;
		var minimize=document.getElementById("minify").checked;
		if(!filename) {
			var i=scripturl.lastIndexOf("/")
			if(i>-1) filename=scripturl.substr();
			else filename=scripturl;	
			var ext=filename.lastIndexOf(".");
			if(ext<0) {ext=filename.length;filename+=".js";}
			filename=filename.substr(0,ext)+"-bundle"+filename.substr(ext);
		}
		var ext=filename.lastIndexOf(".");if(ext<0) {ext=filename.length;filename+=".js";}
		var filenamemin=filename.substr(0,ext)+".min"+filename.substr(ext);
		if(typeof(BUNDLED)==="undefined") makeBundle(scripturl, function(ftext){ 
			var textarea=document.getElementById("tarea");
			//var ftext2=ftext.replaceAll("\n","\r\n");	
			textarea.textContent=ftext;
			//textarea.style.height = textarea.scrollHeight + "px";
			var minimized="";
			if(minimize) 
			{
				minifyClosure(ftext, function (t){
					minimized=JSON.parse(t).compiledCode;		//minimized=t;
					document.getElementById("tareamin").innerText=minimized;
					var element2=document.getElementById("btnmin");
					if(!element2) {
						element2 = document.createElement("input");
						element2.id= "btnmin"
							element2.type = "button";	
						element2.onclick = function() {downloadFile(filenamemin,minimized);};
						document.getElementById("div_e").insertBefore(element2, document.getElementById("div_e").children[0]);				
					} 
					element2.value = "download "+filenamemin;
				});
				document.getElementById("tareamin").innerText="loading...";
			}  

			var element = document.getElementById("btn");
			if(!element){
				element=document.createElement("input");
				element.id= "btn"
					element.type = "button";
				element.onclick = function() {  downloadFile(filename,ftext); };
				document.getElementById("div_e").insertBefore(element, document.getElementById("div_e").children[0]);
			}
			element.value = "download "+filename;

		}); 
		else {
			document.getElementById("div_g").innerText="script bundling started...";
		}
	}

	document.getElementById("gobutton").onclick=func;




}




function minifyClosure(code,cb){
	postURL("https://closure-compiler.appspot.com/compile",
			{
		output_format:	"json",// "text",json
		output_info:"compiled_code",
		
		compilation_level:"SIMPLE_OPTIMIZATIONS", //SIMPLE_OPTIMIZATIONS, WHITESPACE_ONLY
		
		//formatting: "print_input_delimiter",// print_input_delimiter, pretty_print
		//language_out:"ES5",
		//	 output_info:"errors",
		//	 output_info:"warnings",
		//	 warning_level:"default",
		output_info:"compiled_code",
		js_code:encodeURIComponent(code)
		},cb)

}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//postURL
function postURL(url,params,cb){
	function paramToArg(param){
		var arg="";
		for(var k in param) {if(arg) arg+="&";arg+=k+"="+param[k]+"";} 
		return arg;
	}
	var xhr = new XMLHttpRequest();
	xhr.onreadystatechange=function(){if (this.readyState == 4 )cb(xhr.responseText); }
	// xhr.withCredentials = true;
	xhr.open("POST", url, true);

	//Send the proper header information along with the request
	xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
	var p = paramToArg(params);

	console.log("posting request with params:"+p);
	xhr.send(p);
}

/*	closure p="output_format=text\
&output_info=compiled_code\
&compilation_level=SIMPLE_OPTIMIZATIONS\
&js_code=%22console.log('hello')%3B%22";
	 /*p="output_format=json\
&output_info=compiled_code\
&output_info=warnings\
&output_info=errors\
&output_info=statistics\
&compilation_level=SIMPLE_OPTIMIZATIONS\
&warning_level=default\
&output_file_name=default.js\
&js_code=%22console.log('hello')%3B%22";*/


///////////////////////////////////////////////////////////////////////////////////////////////////////
//loadJS
function loadJS(url, callback, location){	// add a script to the dom and call the callback //url is URL of external file, implementationCode is the code to be called from the file, location is the location to insert the element
	if(location==null) location =document.getElementsByTagName('head')[0];
	var scriptTag = document.createElement('script');
	scriptTag.src = url;
	scriptTag.onload = callback;
	scriptTag.onreadystatechange = callback;
	location.appendChild(scriptTag); 
	console.log("loading remote script "+url);
};




/*
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





/* old bundler
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

