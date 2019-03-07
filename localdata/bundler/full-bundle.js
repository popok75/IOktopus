var BUNDLE=true;globalThis.modules={};// added by the bundler
///////////////////////////////////////////////////
// Module : 	module11.js	 //
///////////////////////////////////////////////////
 
    
//require("subfolder/module13");
// require("subfolder/module13.js",function(){console.log("module11 using module 13"+hello13);});
 
 var hello11= "hello world11";
 
 console.log("module11 loaded");
 
// module.exports={hello11};
 ///next line modified by bundler : 'export '= replaced by
globalThis.modules["module11.js"]={};globalThis.modules["module11.js"].exports={hello11}; 

///////////////////////////////////////////////////
// Module : 	module1.js	 //
///////////////////////////////////////////////////
function postmod(m){
	console.log("module1 using module 11 :"+m.hello11);
};
//import("./module11").then((m)=>{postmod(m);});
//require("subfolder/module12");


// import("./module11.js").then(function(someModule) {postmod(someModule);}); 
///next line is commented by the bundler
//import {hello11} from "./module11.js";

// define("module1");
 
 var hello1= "hello world1";
 
 console.log("module1 loaded");
 

///next line modified by bundler : 'export '= replaced by
globalThis.modules["module1.js"]={};globalThis.modules["module1.js"].exports={hello1}; 
//module.exports={hello1};
 

///////////////////////////////////////////////////
// Module : 	module22.js	 //
///////////////////////////////////////////////////
	
 
 function mod22(x, y) {
    return x+y;
};

   
 function mod23(x, y) {
    return x+y;
};
 
console.log("module22 loaded");
  
 ///next line modified by bundler : 'export '= replaced by
globalThis.modules["module22.js"]={};globalThis.modules["module22.js"].exports={mod22};
 
 


///////////////////////////////////////////////////
// Module : 	testapp.js	 //
///////////////////////////////////////////////////
 

console.log("started main script")
console.log("will load external scripts");

 
///next line is commented by the bundler
//import {hello1} from "./module1.js";
///next line is commented by the bundler
//import {mod22} from "./module22.js";
//require("module1",function(){console.log("module1 finished loading")});
 
//require("module22",function(){console.log("module22 finished loading bis")}); 
  

window.onload=function(){	// onload will wait we finish loading modules

	console.log("normal code");
	console.log(hello1);
 	console.log(mod22(1,2));
 //	console.log(hello11);


}


