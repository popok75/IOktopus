 

console.log("started main script")
console.log("will load external scripts");

 
import {hello1} from "./module1.js";
import {mod22} from "./module22.js";
//require("module1",function(){console.log("module1 finished loading")});
 
//require("module22",function(){console.log("module22 finished loading bis")}); 
  

window.onload=function(){	// onload will wait we finish loading modules

	console.log("normal code");
	console.log(hello1);
 		console.log(mod22(1,2));
 //	console.log(hello11);


}


