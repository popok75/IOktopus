function postmod(){
	 console.log("module1 using module 11 :"+hello11);
};
require("module11",postmod);
require("subfolder/module12"); 
   
// define("module1");
 
 var hello1= "hello world1";
 
 console.log("module1 loaded");
 

//export {hello1}; 
module.exports={hello1};
 