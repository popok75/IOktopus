///////////////////////////////////////////////////////////////////////////
//require : will load script into document and track depedencies to fire callback when module is done : 
//e.g. require("script2.js",function(){console.log("script2.js ready");}); 
function require( osrc, cb, async, forcereload ){	// not async by default
	"use strict";	
	function initRequire(){globalThis.require=require; globalThis.modules={};globalThis.deps={};globalThis.depcbs={};}
	function absolutePath(href) {var link = document.createElement("a");link.href = href;return link.href;}
	function hasDependency(src){if(!src) return false; for(var k in deps) for(var j in deps[k]){if(deps[k][j].indexOf(src)>-1) return true;};return false;}
	function newmodule(msrc){
		return {
			get exports(){
				var msrc=document.currentScript.src;
				if(!globalThis.modules[msrc]) return null;
				return globalThis.modules[document.currentScript.src].exports;},	
				set exports(val){
					var msrc=document.currentScript.src;
					if(!globalThis.modules[msrc]) globalThis.modules[msrc]={}; 
					globalThis.modules[msrc].exports=val;
				}	
		};
	};
	function mycb(){//console.log("loaded script :"+src);	 
		if(!hasDependency(src) && cb) cb(globalThis.module,src);		// has no dependencies	
		
		if(!hasDependency(src) && deps[src]) {	 
			var sd=deps[src];delete deps[src];	//call waiting callbacks with no other dependency
			for(var k in sd) if(!hasDependency(sd[k]) && depcbs[sd[k]]) {
				depcbs[sd[k]].forEach(function(cb){cb(globalThis.modules[sd[k]],sd[k]);});
				delete depcbs[sd[k]];// remove called cbs					
			}
		};
	};

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

///////////////////////////////////////////////////////////////////////////
//simple loadJS
function loadJS(src,cb,async){
	var ref=globalThis.document.getElementsByTagName( "script" )[ 0 ];
	var script = document.createElement( "script" );
	script.src = src, script.async = async;
	ref.parentNode.insertBefore( script, ref );
	script.onload = cb;
}












 
 
 
 
 
 
 
 
 
 
 
 
 
/*	Basic require just load, no check

 function require(modulepath,cb){
	 loadJS(modulepath+".js", cb());
 }
 
 function loadJS(url, callback, location){	//url is URL of external file, implementationCode is the code to be called from the file, location is the location to insert the element
	if(location==null) location =document.getElementsByTagName('head')[0];
	var scriptTag = document.createElement('script');
	scriptTag.src = url;

	scriptTag.onload = callback;
	scriptTag.onreadystatechange = callback;

	location.appendChild(scriptTag);
	console.log("loading remote script "+url);
};

*/