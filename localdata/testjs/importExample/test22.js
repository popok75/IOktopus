 /*
 export function mod22(x, y) {
    return x+y;
};*/
 
 	import * as module22 from './module22.js';
 
//let module = await import( './module22.js');
/*
 if(typeof(mod22)=="undefined") {
	import('./module22.js')
  .then((module) => {
    // Faire quelque chose avec le module
	console.log(module.mod22(1,2));
	for(var k in module){
		//export each subcomponent
	if(k=="set" || k=="get") continue;
		globalThis[k]=module[k];
	}
  });
 }*/
window.onload=function(){
 	 console.log(module22.mod22(1,2));
 
console.log("houhou");
}
 
 
 
/*
import('./module22.js')
  .then((module) => {
    // Faire quelque chose avec le module
	console.log(module.mod22(1,2));
  });
	/*
  loadJS('./module22.js',(module) => {
    // Faire quelque chose avec le module
	console.log(mod22(1,2));
  });
  
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
};*/