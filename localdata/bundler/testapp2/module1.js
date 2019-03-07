function postmod(m){
	console.log("module1 using module 11 :"+m.hello11);
};
//import("./module11").then((m)=>{postmod(m);});
//require("subfolder/module12");


// import("./module11.js").then(function(someModule) {postmod(someModule);}); 
import {hello11} from "./module11.js";

// define("module1");
 
 var hello1= "hello world1";
 
 console.log("module1 loaded");
 

export {hello1}; 
//module.exports={hello1};
 