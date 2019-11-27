
function forEachLeaf(data,func){
			for(var i in data) {
				for(var j in data[i]){
					//console.log(typeof(data[i][j]));
					var leaf=false;
					for(var k in data[i][j]) if(typeof(data[i][j][k])=="string") {leaf=true; break;}
					if(leaf) func(i,j);
				}
			}
		};


//function Controller(model0, view0){
class Controller{
	
	 updateNodeControllers(data){
		 
		// remove all controllers not in the new data
		var toremove=[];
		forEachLeaf(this.nodecontrollers, function(i,j){
			if(!(j in data[i])) toremove.push([i,j]);
		});
		for(var k in toremove) {
			var i=toremove[k][0],j=toremove[k][1];
			this.nodecontrollers[i][j].selfRemove();
			delete this.nodecontrollers[i][j];
		}
		// update controllers in the data
		forEachLeaf(data, function(i,j){
			if(this.nodecontrollers[i] && this.nodecontrollers[i][j]){// update existing nodes
				var nc=this.nodecontrollers[i][j];
				nc.update(data[i][j]);
			} else {
				var nc=new NodeController(j,i,data[i][j],this);	//should we clone the data ?
				if(!this.nodecontrollers[i]) this.nodecontrollers[i]={};
				this.nodecontrollers[i][j]=nc; // store nc
			}
		}.bind(this));
		// 
		
	};
	
	
	constructor(model0, view0){
		console.log("Controller");
		this.model=model0;
		this.view=view0;
		this.nodecontrollers={};

		
		this.model.subscribe("load", function(event){// go through the model and check if we have a controller per leaf
			console.log("Controller: got load event from model "+event);
			var data=this.model.data;
			this.updateNodeControllers(data);
			this.view.init();
		/*	forEachLeaf(data, function(i,j){
				var nc=new NodeController(j,i,data[i][j],this);	//should we clone the data ?
				if(!this.nodecontrollers[i]) this.nodecontrollers[i]={};
				this.nodecontrollers[i][j]=nc; // store nc
			}.bind(this));
			*/
		}.bind(this));

		this.model.subscribe("serverinfo",function(event){// go through the model and check if we have a controller per leaf
			console.log("Controller: got serverinfo event from model "+event);
			this.view.updateServerInfo(this.model.serverinfo);
		}.bind(this));

		this.model.subscribe("reload",function(event){// go through the model and check if we have a controller per leaf
			console.log("Controller: got reload event from model "+event);
			var data=this.model.data;
			this.updateNodeControllers(data);
			 
			this.view.update();			
		}.bind(this));
		
		function logupdate(event){ 
			console.log("Controller: got log load/reload event from model "+event);
			this.view.updateLogView(this.model.series,this.model.labels, this.model.data.nodes);
		};
		
		this.model.subscribe("logload", logupdate.bind(this));
		this.model.subscribe("logreload", logupdate.bind(this));
 
		this.model.subscribe("confirmChange", function(event){// go through the model and check if we have a controller per leaf
			console.log("Controller: got confirmChange event from model "+event);
		//	var data=this.model.data;
			for(var p in event){
				var i=0;
				for(var j=0;j<2;j++) i=p.indexOf("/",i+1);
				var p2=p.substr(0,i);
				var nc=deepFind(this.nodecontrollers,p2);
				if(!nc){
					nc=deepFind(this.nodecontrollers,p2);
				}
				var data={};data[p]=event[p]
				nc.react("confirmChange",data);
				console.log(nc);
			} 
		}.bind(this));
			 
		this.model.subscribe("cancelChange",this.view.notifyCancel.bind(this.view));

		this.view.subscribe("init",function(event){
			console.log("Controller: got event from view "+event);			
		}.bind(this));
		};	

	};

	function deepFind(obj, path){
	    for (var i=0, path=path.split('/'), len=path.length; i<len; i++){
	        obj = obj[path[i]];
	    };
	    return obj;
	};

	class NodeController {
		constructor(name, directory, data, controller){
			//console.log("NodeController");
			// build view components matching data content
			// in group : directory
			// in subgroup : type
			// a node view that display : name, value, minmax if there is
			var ggd=globalThis.globaldefs.data;
			this.group=directory;
			this.subgroup=data[ggd.typefield] || data[ggd.classfield] || " ";
			this.nodeview= controller.view.createNode(name,this.group,this.subgroup,data);
				//subgroup and typefield/classfield not used/implemented ?
			this.path=directory+"/"+name;
			this.controller=controller;
			this.nodeview.subscribe("userChange",function(event){this.react("userChange",event);}.bind(this));
			this.name=name;
		};	 

		selfRemove(){
			this.controller.view.removeNode(this.nodeview, this.name,this.group,this.subgroup);
		}
		
		update(nodedata){
			this.nodeview.update(nodedata);
		}

		react(eventname, eventdata){
			console.log();
			if(eventname=="userChange") {for(var k in eventdata)	this.controller.model.userChange(k,eventdata[k]);}
			
			if(eventname=="confirmChange") {
				for(var p in eventdata){
					var p2=p.substr(this.path.length+1),oe={};	// keep only subpath
					oe[p2]=eventdata[p];
					this.nodeview.update(oe,true);					
				}
			}
		}

	};
	
	function pathToObj(po){
		var fobj={};
		for(var p in po){
			var tab=p.split("/"),tfobj=fobj,lastkey,ptfobj=tfobj;
			for(var k in tab){
				lastkey=tab[k];
				tfobj[lastkey]={};
				ptfobj=tfobj;
				tfobj=tfobj[lastkey]
			}
			ptfobj[lastkey]=po[p];
		}
		return fobj;
	}

	export {Controller};