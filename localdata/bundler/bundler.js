//- loadremote+sequencializer
//- parser
//- find : import/export, loadJS
//- jump comments
//- exclude/include tags e.g. //bundler-exclude-nextline// //bundler-exclude-start// //bundler-exclude-end//
//- process
//- download
//- order
//- rewrite : remove import/export, load






// Make a bundle from the script in url and load subsequent require/imports


function makeBundle(url,cb){
	var SubParsers={
			parsers:[getRequireModules,getImportModules],
			removers:[removeRequireModules,removeImportModules,removeExcludes],	
			perscript:[correctRequireExports,correctImportExports],
			comments: [["//bundler-include-start//","//bundler-include-end//"],["//","\n"],["/*","*/"]]
	};
	var textmap={}, pathorder=[], requested=0;
	function getSubModules(text,url,parsers, comments){	//return an array of file names used as modules in require or import
		var res=[];
		if("http://192.168.2.3:8080/js/ui/dygraph.min.js"==url) {
			console.log("hello");
		}
		text=removeComments(text);	//no need
		for(var k in parsers) res=res.concat(parsers[k](text,url,comments));
		res=Array.from(new Set(res))
		return res;
	}
	function parseBundle(t,xhttp){
		textmap[xhttp.responseURL]=t;
		var modules=getSubModules(t,xhttp.responseURL,SubParsers.parsers,SubParsers.comments);
		var root=xhttp.responseURL.substr(0,xhttp.responseURL.lastIndexOf("/")+1);
		for(var m in modules){
			var murl=modules[m]
			if(pathorder.indexOf(murl)==-1) {sequencer(loadRemote,1, murl,parseBundle);requested++;}
			insertPathBefore(murl,xhttp.responseURL);
		}
		requested--;
		if(requested==0) {
			assembleBundle(textmap,pathorder,cb,SubParsers.removers,SubParsers.perscript,SubParsers.comments);
		}
	}
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
	
//	var absurl=getAbsUrlFrom(url,document.scripts[0].src);
//	pathorder.push(absurl);
	pathorder.push(new URL(name,url).href);
	sequencer(loadRemote,1,url,parseBundle);requested++;
 
}

 
/*
function getAbsUrlFrom(relativepath,basepath){
	var l=basepath.lastIndexOf("/");
	return getAbsUrl(relativepath,basepath.substr(0,l+1));
};

function getAbsUrl(relativepath,path){
	if(relativepath.indexOf("//")>-1) return relativepath;	//is already absolute
	 ;
	if(relativepath.substr(0,2)=="./") relativepath=relativepath.substr(2);
	while(relativepath.substr(0,3)=="../") {
		relativepath=relativepath.substr(3);
		if(path[path.length-1]=="/") path=path.substr(0,path.length-1);
		var last=path.lastIndexOf("/");
		path=path.substr(0,last+1);
	}
	return path+relativepath;
};
*/

function assembleBundle(map,order,cbf,removers,perscriptcorrector,comments){
	function getHeaderComment(scripturl,starturl){
		var bname=starturl.substr(0,starturl.lastIndexOf("/")+1), sname=scripturl.substr(bname.length) ;
		return "///////////////////////////////////////////////////\n// Module : \t"+ sname + "\t //\n///////////////////////////////////////////////////\n";
		}
	var bundled="";
	for(var l in order) {// we use the right order of the loaded scripts
		var modulename=order[l].substr(order[l].lastIndexOf("/")+1),modulepath=order[l].substr(order[l].lastIndexOf("/"));
		if(modulename=="bundler" || modulename=="require") continue;				
		var comment=getHeaderComment(order[l],order[order.length-1]);
		
		if(bundled) comment="\n\n"+comment;
		var moduletext=map[order[l]];
		for(var k in perscriptcorrector) moduletext=perscriptcorrector[k](moduletext,order[l],order[order.length-1]);
		bundled+=comment+moduletext;
	}
	for(var r in removers) bundled=removers[r](bundled,comments);
 
 	var header="var BUNDLE=true;globalThis.modules={};// added by the bundler\n";		
 
	bundled=header+bundled;
	if(cbf) cbf(bundled);
}

 


 
 
function getRequireModules(ctext,url,comm){	//return an array of file names used as modules in require or import
	var res=[], keyword="require(\"",pathorder=[];
	var j=indexOfSkipping(ctext,keyword,0,comm);
	while(j>-1){
		j=j+keyword.length-1;		 
		var k=ctext.indexOf(");",j);	// ); at end if require is needed to parse correctly, without counting parenthesis
		if(k<j) break;
		var line=ctext.substr(j,k-j),l=line.indexOf(",");
		if(l>-1) line=line.substr(0,l);
		console.log("evaluating :"+line);
		var ppath=eval(line);
		ppath+=".js";
		
//		res.push(getAbsUrlFrom(ppath,url));
		res.push(new URL(ppath,url).href);
		j=indexOfSkipping(ctext,keyword,j,comm);
	}
	return res;
}
 




function removeRequireModules(text,comments){	//return an array of file names used as modules in require or import	
	var text2=text, option=2;
	if(indexOfSkipping(text2,"require(\"",0,comments)>-1) {
		if(option==1)// option 1 comment out requires. exports are not important
			text=replaceAllSkipping(text,"require(\"","///next line is commented by the bundler\n//require(\"");		 
		 else if(option==2 && indexOfSkipping(text2,"require(\"",0,comments)>-1) // option 2 redefine require to do nothing
			text="function require(m,cb){if(cb) cb();};\n"+text;		
	}	
	return text;
}


function correctRequireExports(text,scripturl,starturl){	//return an array of file names used as modules in require or import
	var bname=starturl.substr(0,starturl.lastIndexOf("/")+1), sname=scripturl.substr(bname.length) ;
//	text=text.replaceAll("module.exports=","//next line modified by bundler : 'module.exports'= replaced by\nglobalThis.modules[\""+sname+"\"]={};globalThis.modules[\""+sname+"\"].exports=");
	text=replaceAllSkipping(text,"require(\"","///next line is commented by the bundler\n//require(\"",[["//","\n"],["/*","*/"]]);
	
	return text;
}

function getImportModules(text,url,comm){	//return an array of file names used as modules in require or import
	//find import, extract filename, use url as base
	function parseScriptName(script,pos){
		var c="\"";
		var k=script.indexOf("\"",pos);
		var k2=script.indexOf("'",pos);
		if(k2>-1 && k2<k) {k=k2;c="'";}
		if(k<0) return "";
		var l=script.indexOf(c,k+1);
		if(l<0) return "";
		return script.substr(k+1,l-k-1);
	};
	var keyword="import", res=[];
	var i=indexOfSkipping(text,keyword,0,comm);
	while(i>-1){
		var name=parseScriptName(text,i);
		if(name=="/log/") {
			parseScriptName(text,i);
		}
//		res.push(getAbsUrlFrom(name,url));
		res.push(new URL(name,url).href);
		i=indexOfSkipping(text,keyword,i+1,comm);
	}
	return res;
}

function removeImportModules(text,comments){	//return an array of file names used as modules in require or import
	var text2=text;	
 	if(indexOfSkipping(text2,"import",0,comments)>-1)
 		text=replaceAllSkipping(text,"import","///next line is commented by the bundler\n//import",comments);
//			text=text.replaceAll("import {\"","//next line is commented by the bundler\n//import {\"");
 	
	return text;
}
function correctImportExports(text,scripturl,starturl){	//return an array of file names used as modules in require or import
	var bname=starturl.substr(0,starturl.lastIndexOf("/")+1), sname=scripturl.substr(bname.length) ;
	// text=text.replaceAll("export ","//next line modified by bundler : 'export '= replaced by\nglobalThis.modules[\""+sname+"\"]={};globalThis.modules[\""+sname+"\"].exports=");
	 text=replaceAllSkipping(text,"export ","///next line modified by bundler : 'export '= replaced by\nglobalThis.modules[\""+sname+"\"]={};globalThis.modules[\""+sname+"\"].exports=",[["//","\n"],["/*","*/"]]);
	return text;
}


function removeExcludes(text){
	var i=text.indexOf("//bundler-exclude-nextline");	
	while(i>-1){
		var j=text.indexOf("\n",i+1);
		if(text[j]=="\r") j++;
		if(j>-1) text=text.substr(0,j+1)+"//"+text.substr(j+1);
	}
	text=text.replaceAll("//bundler-exclude-start//","//bundler-exclude-start//\n/*")
	text=text.replaceAll("//bundler-exclude-end//","*///bundler-exclude-end//")
 
	return text;
}

 


function removeComments(t){
	var t2=t;
	var i=0;
	while( i> -1)
	{
		i=t.indexOf("/*",i);
		if(i>-1) {
			var j=t.indexOf("*/",i+1);
			if(j>-1) t=t.substring(0,i)+t.substring(j+2);
		}
	}
	
	i=0;
	while( i> -1)
	{
		i=t.indexOf("//",i);
		if(i>-1) {
			var j=t.indexOf("\n",i+1);
			if(j>-1) t=t.substring(0,i)+t.substring(j+1); else t=t.substr(0,i);
		}
	}

	return t;
}





function indexOfSkipping(text,key, startpos, comm){ //search index while skipping comments
	function skipComments(text, index,startindex,comm){				// find if index is inside a comment, if true position return after this comment 
		for(var c in comm){	//for each comment type
			var cont=true;
			while(cont){	// do repeatedly until ci0>index or c1>index
				var ci0=text.indexOf(comm[c][0],startindex);	// search comment start
				if(ci0==-1 || ci0>=index) {cont=false;break;}
				if(ci0>-1 && ci0<index){	// if comment is before index
					var ci1= text.indexOf(comm[c][1], ci0+comm[c][0].length);	// search comment end
					if(ci1==-1) return text.length;
					if(ci1>index) return ci1+comm[c][1].length;					// if end is after index return
					startindex=ci1+comm[c][1].length;		// position new start after comment end
				}				
			}
		}
		return index;	// no occurrence found
	}
	
	var i=startpos+1,j=startpos;
	while(i!=j){
		i= text.indexOf(key,j);
		if(i==-1) return -1;
		j=skipComments(text, i,j,comm);
		if(j<0) return -1;
	}
	
	return i;
}

function replaceAllSkipping(text,search, replacement,comm) {
	var target = text,i=0;
    while((i=indexOfSkipping(target,search,i,comm))>-1) {
    	target=target.substr(0,i)+replacement+target.substr(i+search.length);//target=target.replace(search,replacement);
    	i+=replacement.length;
    }
    return target;
//	var target = this;
 //   return target.replace(new RegExp(search, 'g'), replacement);
};




String.prototype.replaceAll = function(search, replacement) {
	var target = this,i=0;
    while((i=target.indexOf(search,i))>-1) {
    	target=target.substr(0,i)+replacement+target.substr(i+search.length);//target=target.replace(search,replacement);
    	i+=replacement.length;
    }
    return target;
//	var target = this;
 //   return target.replace(new RegExp(search, 'g'), replacement);
};


/////////////////////////////////////////////////////////////
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

//sequencer calls a function (e.g. remoteload) in a sequencial manner: 
//until callback of one is not called, the function (e.g. remoteload) is not called for next item
var waitseq={},loadingitem=false;
function sequencer(func,cbindex){  //func is the function to call, cbindex is the index of the callback to substitute in the arguments of func
	var args=[];
	for(var k in arguments) args[k]=arguments[k];// make an object with arguments
	args.splice(0,2);	//remove first arguments
	var origcb=args[cbindex];
	function mycb(){		 
		console.log("bundler loaded "+arguments[1].responseURL);
		//	console.log("splicing args for func "+waitseq[func][0]);
		//var nommore=true;
		origcb(...arguments);
		if(waitseq[func]) {
			console.log("sequencer loading delayed "+waitseq[func][0][0]);
			func(...waitseq[func][0]);
			waitseq[func].splice(0,1);
			if(waitseq[func].length==0) delete waitseq[func];
		} else loadingitem=false;
	};
	args[cbindex]=mycb;
	 
	if(!loadingitem) {
		loadingitem=true;
		console.log("sequencer loading now " +args[0]);
		func(...args);
	} else {
		console.log("sequencer rescheduling for later " +args[0]);
		if(args[0]=="http://192.168.2.3/js//log/") {
			console.log("problem here");
		}
		if(!waitseq[func]) waitseq[func]=[];
		waitseq[func].push(args);
	}	
	
//	if(waitseq[func]) {waitnow=true;}//if we started this function before and waiting for it to end
//	else waitseq[func]=[];// save arguments
	//console.log("adding args to func "+args);
}



//loadRemote : load a remote addr and call cb with it's content+xhttp.  retries can be specified
function loadRemote(addr,cb,opt){	// opt :{initf:function(xhttp){}, retryCallback:function(response,xhttp){}, retries:Number()}		
	console.log("loadRemote addr "+addr);
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
 
	xhttp.open("GET", addr, true);
	if(opt && opt.initf) opt.initf(xhttp);
	xhttp.send();
};







