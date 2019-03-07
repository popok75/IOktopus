
//loadRequireinit();


console.log("started main script")
console.log("will load external scripts");

 
require("module1",function(){console.log("module1 finished loading")});
 
require("module22",function(){console.log("module22 finished loading bis")}); 
 
//loadJS("require.js",initmodules);
/*import * as req from './require.js';// if bundled should be replaced by var req={}
expmod(req);
initmodules();
//import("./require.js").then(expmod);



//
function expmod(mod){
	for(var k in mod){
		//export each subcomponent
		if(k=="set" || k=="get") continue;
		globalThis[k]=mod[k];
	};
}

function initmodules() {
	require("module1",function(module){
		console.log(module);
	});
	require("module11");
};

 */


window.onload=function(){	// onload will wait we finish loading modules
	/*	initmodules(function(){
		console.log(hello1);
	//	console.log(hello11);

	});
	console.log("onload");

	/* 	seqIncludeScript("module11.js");
	 	seqIncludeScript("subfolder/module12.js");
	 	seqIncludeScript("subfolder/module13.js");
	 */		

	// all scripts loaded at this point (using require or something else) will be bundled

	console.log("normal code");
	console.log(hello1);
		console.log(mod22(1,2));
	console.log(hello11);


}











//bundler-exclude-start//
///////////////////////////////////////////////////////////////////////////
//require : will load script into document and track depedencies to fire callback when module is done : 
//e.g. require("script2.js",function(){console.log("script2.js ready");}); 
function require( osrc, cb, async, forcereload ){	// not async by default
	"use strict";	
	function initRequire(){globalThis.require=require; globalThis.modules={};globalThis.deps={};globalThis.depcbs={};}
	function absolutePath(href) {var link = document.createElement("a");link.href = href;return link.href;}
	function hasDependency(src){if(!src) return false; for(var k in deps) for(var j in deps[k]){if(deps[k][j].indexOf(src)>-1) return true;};return false;}
//	function newmodule2(){return {get exports(){if(!globalThis.modules[document.currentScript.src]) return null;return globalThis.modules[document.currentScript.src].exports;},	
//									 set exports(val){var msrc=document.currentScript.src;if(!globalThis.modules[msrc]) globalThis.modules[msrc]={};globalThis.modules[msrc].exports=val;}};};								 									 
	function newmodule() {var o={};
		Object.defineProperty(o, "exports", {
			get : function(){if(!globalThis.modules[document.currentScript.src]) return null;return globalThis.modules[document.currentScript.src].exports;},
            set : function(val){var msrc=document.currentScript.src;if(!globalThis.modules[msrc]) globalThis.modules[msrc]={};globalThis.modules[msrc].exports=val;}});
		return o;
	};
									 
	function mycb(){//console.log("loaded script :"+src);	 
		if(!hasDependency(src) && cb) cb(globalThis.module,src);		// has no dependencies		
		if(!hasDependency(src) && deps[src]) {	 
			var sd=deps[src];delete deps[src];	//call waiting callbacks with no other dependency
			for(var k in sd) if(!hasDependency(sd[k]) && depcbs[sd[k]]) {
				depcbs[sd[k]].forEach(function(cb){cb(globalThis.modules[sd[k]],sd[k]);});
				delete depcbs[sd[k]];// remove called cbs					
		}};};
	if(!globalThis.require) initRequire();
	osrc+=".js";
	var src=absolutePath(osrc), scripts=globalThis.document.getElementsByTagName( "script" );
	var deps=globalThis.deps,current=document.currentScript	? document.currentScript.src : "";		// if no callback no dependency saving ?
	if(!deps[src]) deps[src]=[];		// save dependency  
	deps[src].push(current);
	var noload=false;
	if(!forcereload) for(var k in scripts) if(scripts[k].src==src){if(globalThis.modules[src]) {mycb();return;} else noload=true;}	// if already loaded fire cb, if not loaded yet do not reload
	if(typeof(cb)==="function") {if(!depcbs[src]) depcbs[src]=[]; depcbs[src].push(cb);}	// save callback
	if(noload) return;
	globalThis.module=newmodule();		
	globalThis.modules[src]={};
	
	loadJS(src,mycb,async);
	return globalThis.modules[src];
};
//bundler-exclude-end//
///////////////////////////////////////////////////////////////////////////
//simple loadJS
function loadJS(src,cb,async){
	var ref=globalThis.document.getElementsByTagName( "script" )[ 0 ];
	var script = document.createElement( "script" );
	script.src = src, script.async = async;
	ref.parentNode.insertBefore( script, ref );
	script.onload = cb;
}


/*function loadJS(url, callback, location,forceaddagain){	// add a script to the dom and call the callback //url is URL of external file, implementationCode is the code to be called from the file, location is the location to insert the element
// must complete the url first
var head=document.getElementsByTagName('head');
if(!forceaddagain) for(var k in head) if(head[k].src==url) {callback();return;}	//already loaded 
if(!location) location =document.getElementsByTagName('head')[0];
var scriptTag = document.createElement('script');
scriptTag.src = url;
scriptTag.onload = callback;
scriptTag.onreadystatechange = callback;
location.appendChild(scriptTag); 
console.log("loading remote script "+url);
};
 */








/*
// This script is a homebrew require, very simple and allowing sequential loading of modules, so useful for me
  console.log("require module initializing");

///////////////////////////////////////////////////////////////////////////////////////////////////////
 // 	loadJS
function loadJS(url, callback, location){	// add a script to the dom and call the callback //url is URL of external file, implementationCode is the code to be called from the file, location is the location to insert the element
	if(location==null) location =document.getElementsByTagName('head')[0];
	var scriptTag = document.createElement('script');
	scriptTag.src = url;
	scriptTag.onload = callback;
	scriptTag.onreadystatechange = callback;
	location.appendChild(scriptTag); 
	console.log("loading remote script "+url);
};
 
// load scripts sequencially not in parallel// when it is busy loading a script just pile it
var scriptswaiting, scriptbusy;
function seqLoadJS(url, callback){
	if(typeof(scriptswaiting)==="undefined") {
		scriptswaiting=[], scriptbusy=false;//initialize undeclared globals
	}
	if(scriptbusy) {scriptswaiting.push([url,callback]);return;}
	var func=function(){//console.log("dowloaded script");
		scriptbusy=false;
		if(scriptswaiting.length>0) {			//var fr=scriptswaiting.front();
			var tab=scriptswaiting.splice(0,1)[0];
			scriptbusy=true;//	console.log("requesting script "+fr);
			loadJS(tab[0],func.bind(tab[1]));}
		if(this) this();
	};
	scriptbusy=true;//	console.log("requesting script "+url);
	loadJS(url,func.bind(callback));	
}

function fileName(path){ 
	 var i=path.lastIndexOf("/");
	 if(i<0 || i>=(path.length-1)) i=0;
	 var sub=path.substr(i+1);	 
	return sub;
}







 ///////////////////////////////////////////////////////////////////////////////////////////////////////
// 	require & define 
var modules;
 function define(modulepath){	// save module name 
	 if (typeof modules == 'undefined') modules={defined:[]};
	 if(defined(modulepath)) return;
 	 modules.defined.push(moduleName(modulepath));
 }
 
 function defined(modulepath){	// check if module name is saved as defined
	var modulename=moduleName(modulepath);
	 if (typeof modules !== 'undefined')  for(var k in modules.defined) {if(modulename==modules.defined[k]) return true;}	//already loaded
	 return false;
 }


 
function moduleName(modulepath){ //remove the .js 
	 var sub=fileName(modulepath),j=sub.lastIndexOf(".");
	 if(j>-1 && sub.substr(j)==".js") sub=sub.substr(0,j);
	return sub;
 }

function modulePath(modulepath){ 
	 var p=modulepath.indexOf("://");
	 if(p<0) p=-3;
	 var i=modulepath.indexOf("/",p+3);
	 if(i<0 || i>=(modulepath.length-1)) return modulepath;
	 var j=modulepath.lastIndexOf("/");
	return modulepath.substr(i+1,j-i);
}

var module=globalThis.module={};
 function require(modulepath,cb){	// load module unless aleady defined
//	 if (typeof module === 'undefined') module=globalThis.module={};
	 if(defined(modulepath)) {if(cb) cb();return;}
	 var func=function(){	
			define(modulepath);
			if(cb) cb(module.exports);		//not good : called before all sub dependency are loaded
			 };
	 seqLoadJS(modulepath+".js",func);
 }
 


 module.exports={require,define};
 export {require,define};


 
 
 
 */











