
console.log("started script")
console.log("will load external scripts");

loadJS("require.js",initmodules);
 


function initmodules(){
 
	 require("module1");
	 require("module11");
	 require("subfolder/module12");
	 
	 console.log("initialized all modules");
}
 
 

 
 window.onload=function(){	// onload will wait we finish loading modules
		console.log("onload");
		
	/* 	seqIncludeScript("module11.js");
	 	seqIncludeScript("subfolder/module12.js");
	 	seqIncludeScript("subfolder/module13.js");
*/		
		 
		// all scripts loaded at this point (using require or something else) will be bundled
	 	 
		console.log("normal code");
		console.log(hello1);
		console.log(hello11);
		console.log(hello12);
 

 }
 
 
 
 
 
 
 
 
 
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
 

 

 