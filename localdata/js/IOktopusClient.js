
import {} from "./globalexport.js";		// defines globalexport({objs,funcs}) that export anything to global 
//to be compatible with other type and without copy the instance created by the module than es6 module system (e.g. loadJS)

import {} from "./globaldefs.js";		

//import {loadremote,loadFav} from './loadremote.js';
import {} from './loadremote.js';

import {RemoteModel} from "./RemoteModel.js";
import {UserView} from "./ui/UserView.js";
import {Controller} from "./Controller.js";



var geid=document.getElementById.bind(document);




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//loading doc


function initclient() {
	var model=new RemoteModel();
	var view=new UserView();
	var controller=new Controller(model,view);
	model.init();
	geid('userinterface').innerHTML="client initialized";
	geid('title').innerHTML="IOktopus v0.3 growing...";
}




var multifile=true;







//Basic window on load
window.onload = function() {
	loadFav('/favicon.png?v1',function(){});
	loadBottomImage(function(){});
	initclient();
//	if(multifile) loadJSScripts(initclient);	// for dev purpose each class is its own file
//	else initclient();							// for production, one file is made containing all the classes

	/*loadBottomImage(function(){
	loadFav(function(){loadAllModel();});
	});*/

	//	<img src='/favicon.png' alt='IOktopus' style='width:120px;'>
	//<link rel='icon' href='/favicon.png' sizes='32x32'> 
};





function loadBottomImage(callback) {
	var img=document.createElement('img');
	img.src='/favicon.png' ;
	img.alt='IOktopus'; 
	img.style='width:120px;'; 
	img.onload = callback;
	document.getElementsByTagName('body')[0].appendChild(img );
}
