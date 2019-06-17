


//function Controller(model0, view0){
class Controller{
	constructor(model0, view0){
		console.log("Controller");
		this.model=model0;
		this.view=view0;
		this.nodecontrollers={};

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
		
		this.model.subscribe("load",function(event){// go through the model and check if we have a controller per leaf
			console.log("Controller: got load event from model "+event);
			var data=this.model.data;
			forEachLeaf(data, function(i,j){
				var nc=new NodeController(j,i,data[i][j],this);	//should we clone the data ?
				if(!this.nodecontrollers[i]) this.nodecontrollers[i]={};
				this.nodecontrollers[i][j]=nc; // store nc
			}.bind(this));
			this.view.init();
		}.bind(this));

		this.model.subscribe("serverinfo",function(event){// go through the model and check if we have a controller per leaf
			console.log("Controller: got serverinfo event from model "+event);
			this.view.updateServerInfo(this.model.serverinfo);
		}.bind(this));

		this.model.subscribe("reload",function(event){// go through the model and check if we have a controller per leaf
			console.log("Controller: got reload event from model "+event);
			var data=this.model.data;
			forEachLeaf(data, function(i,j){
				if(this.nodecontrollers[i] && this.nodecontrollers[i][j]){// update existing nodes
					var nc=this.nodecontrollers[i][j];
					nc.update(data[i][j]);
				}
			}.bind(this));
			this.view.update();			
		}.bind(this));
			
		this.model.subscribe("logload",function(event){ 
			console.log("Controller: got logload event from model "+event);
		//	this.view.createLogView(JSON.parse(JSON.stringify(this.model.log.slice())), this.model.data.nodes);
			this.view.updateLogView(this.model.series,this.model.labels, this.model.data.nodes);
		}.bind(this));
		this.model.subscribe("logreload",function(event){ 
			console.log("Controller: got logreload event from model "+event);
			this.view.updateLogView(this.model.series,this.model.labels);	// should include nodes in case of an update
		}.bind(this));

			
		this.view.subscribe("init",function(event){
			console.log("Controller: got event from view "+event);			
		}.bind(this));
	};	

};


class NodeController {
	constructor(name, directory, data, controller){
		console.log("NodeController");
		// build view components matching data content
			// in group : directory
			// in subgroup : type
			// a node view that display : name, value, minmax if there is
		this.nodeview= controller.view.createNode(name,directory,data["type"] || data["class"] || "other",data);
		 
		this.controller=controller;
		this.nodeview.subscribe("updateValue",function(event){this.react(event);}.bind(this));
		
	};	 
	
	update(nodedata){
		this.nodeview.update(nodedata);
	}
	
	react(event){
		console.log();
		for(var k in event)	this.controller.model.userChange(k,event[k]);
	};
	
};


export {Controller};