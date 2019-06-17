


class Component{
	constructor(name=""){
		this.components=[];
		this.name=name;
		//	console.log("Component");
	}

	addComponent(comp){
	//	if(!name) name=comp.name;
		if(comp==undefined || comp.name==undefined){
			//problem
			console.log("problem");
		}
		this.components.push(comp);
	}
	addComponentAlphabeticOrder(comp,name=0){
		if(!name) name=comp.name;
		if(comp==undefined || comp.name==undefined){console.log("problem");}//problem
		for(var i in this.components){
			if(this.components[i].name>name) {this.components.splice(i,0,comp);return;} 
		}
		this.components.push(comp);
		}

	getComponent(name){for(var i in this.components){if(this.components[i].name==name) return this.components[i];}return null;}//		return this.components[name];
		
	hasComponent(name){for(var i in this.components){if(this.components[i].name==name) return true;}return false;}

	getHtml(){	
		var html="";
		if(this.pretag)	html+=this.pretag;
		if(this.tag) {
			html+="<"+this.tag;
			if(this.name) html+=" id='"+this.name+"'";
			if(this.style) html+=" style='"+this.style+"'";
			if(this.extra) html+=" "+this.extra;	
			html+=">";
		}
		html+=this.getChildrenHtml();
		if(this.tag) html+="</"+this.tag+">";
		if(this.posttag) html+=this.posttag;
		return html;
	} //return html of this component and children

	getChildrenHtml(){
		var html="";
		for(name in this.components) {
			html+=this.components[name].getHtml();
		}
		return html;
	}

	findByName(tofind){	//recursive find component
		for(var i in this.components) {
			if(this.components[i].name==tofind) return this.components[i];
			var obj= this.components[i].findByName(tofind);
			if(obj) return obj;
		}
		return undefined;
	}

	updateProperty(propname,propvalue){
		var item=geid(this.name);
		if(item) item[propname]=propvalue;
	}
	
	updateStyle(subpropname,propvalue){
		var item=geid(this.name);
		if(item) item.style[subpropname]=propvalue;
	}
}




class Panel extends Component{
	constructor(name,style,caption){
		super(name,style);
		//not compatible with style etc :(
		var pretable="<div><table id='"+this.name+"' class='pure-table pure-table-bordered pure-table-striped'";
		if(style) pretable+=" style='"+style+"'";
		if(caption !=undefined) pretable+="><thead><caption>"+caption+"</caption></thead> <tbody><tr><td>";
		else pretable+="><thead><caption>"+this.name+"</caption></thead> <tbody><tr><td>";
		var posttable=" </td></tr></tbody></tbody> </table><div>";
		var prefield="<div id='"+this.name+"';" ;
		if(style) prefield+=" style='"+style+"'";
		if(caption!=undefined) prefield+="><fieldset id='"+this.name+"fieldset' class='pure-fieldset'><legend>"+caption+"</legend>";
		else prefield+="><fieldset id='"+this.name+"fieldset' class='pure-fieldset'><legend>"+this.name+"</legend>";
		var postfield="</fieldset></div>";
		if(name){this.pretag=prefield;this.posttag=postfield;}	// no name, no html code

	};

	addPanel(name,style){
		var newpanel = new Panel(name,style);
		this.addComponent(newpanel);
		return newpanel;
	}

};

class Table extends Component{
	constructor(name){
		super(name);
		this.pretag="<div><table id='"+this.name+"' class='pure-table pure-table-bordered pure-table-striped'><thead><caption>"+this.name+"</caption></thead> <tbody>";
		this.posttag="</tbody></table></div>";

	};

};
class TableRow extends Component{
	constructor(name){super(name);this.tag="tr";};
}

class TableCell extends Component{
	constructor(name){super(name);this.tag="td";};
}


class DivComponent extends Component{
	constructor(name){super(name);this.tag="div";};
}


class TextLabel extends Component{
	constructor(name,text=""){
		super(name);
		this.text=text;
		this.fontsize=1;
	};
	getHtml(){
		var html="<div id='"+this.name+"'";
		if(this.style) html+=" style='"+this.style+"'";
		html+=">";
		if(this.fontsize!=1) html+="<font style='font-size:"+100*this.fontsize+"%'>";
		html+=this.text;
		if(this.fontsize!=1) html+="</font>";
		html+="</div>";
		//	console.log("getHtml():"+html);
		return html
		;
	}

	update(text){
		this.text=text;
		document.getElementById(this.name).outerHTML = this.getHtml();
	}
}

class SelectComponent extends Component{
	constructor(name,entries,selectedindex=0){
		super(name);
		this.entries=entries;
		this.selected=selectedindex;
		this.style="";
		this.extra=" onchange='objectChange(this);'";
		subscribeObject(this.name,this);
	};
	
	changed(name,val){
		if(this.notify) if(!this.notify(this.entries[val])) return ;
		//console.log("SelectComponent changed");
		this.selected=val;
		if(this.postnotify) this.postnotify(val);
		
	}
 
	getHtml(){
		var html="<select id='"+this.name+"'";
		if(this.style) html+=" style='"+this.style+"'";
		if(this.extra) html+=this.extra;
		html+=">";
		for(var t in this.entries){
			var n=this.entries[t];
			html+= "<option value='"+t+"'";
			if(t==this.selected) html+=" selected"; 
			html+=">"+n+"</option>";	
		}
		
		html+="</select>";
		if(this.posttag) html+=this.posttag;
		//	console.log("getHtml():"+html);
		return html
		;
	}

	update(text){
		this.text=text;
		document.getElementById(this.name).outerHTML = this.getHtml();
	}
}

function delayEvent(o){
	setTimeout(function(){objectChange(o);},50);
	 
};

class TextAreaComponent extends Component{
	constructor(name,initval, initwidth){
		super(name);
		this.text=initval;	
		this.style="";
		this.tag="input"
		this.extra=" type='text' value='"+this.text+"' onchange ='objectChange(this);' onkeydown='delayEvent(this)' onpaste='delayEvent(this)' oncut='delayEvent(this)'  onKeyPress ='delayEvent(this)' onKeyUp ='delayEvent(this);'";
		subscribeObject(this.name,this);
		if(initwidth) {
			this.extra+="size="+initwidth;
			this.style+="width:"+initwidth+"em;";
			}
	};
	
	changed(name,value){
		var val=geid(this.name).value;
		if(this.text==val) return;
		if(this.notify) if(!this.notify(val)) return;
		this.text=val;
		if(this.postnotify) this.postnotify(val);
	}
 
	update(text){
		this.text=text;
		document.getElementById(this.name).outerHTML = this.getHtml();
	}
}










/////////////////////////////////////////////////////////////////////////
var ComponentFactory={
		create(compname, compclass, comptype,arg4,arg5,arg6){
			if(compclass=="BinaryComponent") {
				if(comptype=="labels") return new LabelBinaryComponent(compname,arg4,arg5,arg6);
				if(comptype=="select") return new SelectBinaryComponent(compname,arg4,arg5,arg6);
				
			}
		}
};

class BinaryComponent extends Component{
	constructor(name){
		super(name);
		this.state=0;
	};	 
}

var objlist={};
function objectChange(o) {
	var e=o.id;	
	var target=objlist[e];
	if(target) target.changed(o.id,o.value);
//	return alarms[e.n]["al"+e.id].confirm(name,val);
}

function subscribeObject(name, obj){
	objlist[name]=obj;
} 

var lightgray="#BBBBBB";
 
class SelectBinaryComponent extends BinaryComponent{
	constructor(name,offtext,ontext){
		super(name);
		this.content=new SelectComponent(name+"content",[offtext,ontext]);
		this.addComponent(this.content);
		this.tag="div";
		this.style="display:inline-block;cursor: pointer;";
		this.content.style="font-size:inherit;cursor:inherit;";
		//if(!this.content.extra) this.content.extra="";
		 
		this.extra+=" onchange='objectChange(this)'";
		subscribeObject(this.name,this);
		this.state=false;
	}	
	
	changed(name,value){
		if(this.notify && !this.notify()) return;
		this.state=geid(this.name).children[0].children[1].selected;
		if(this.postnotify) this.postnotify(); 
	}
}; 


class LabelBinaryComponent extends BinaryComponent{
	constructor(name,offtext,ontext,opt){
		if(!opt) opt={border:true,usegray:true};
		super(name);
		this.offlabel=new TextLabel(this.name+"off",offtext);
		this.onlabel=new TextLabel(this.name+"on",ontext);
		this.onlabel.style="display: none;"
		this.style="cursor: pointer;display:inline-block;";
		if(opt.border) {this.style+="border-style: solid;border-width: 1px;";}
			
		if(opt.usegray) {
			this.style+="color:"+lightgray+";";
			this.offlabel.style="color:"+lightgray+";";
		}
		this.tag="div";
		this.extra=" onclick='objectChange(this)'";
		subscribeObject(this.name,this);
		this.addComponent(this.offlabel);
		this.addComponent(this.onlabel);
		this.opt =opt;
	};

	changed(name,value){
		console.log("Changed ! "+name+" "+value);
		if(this.notify && !this.notify(this)) return;
		this.changeState();
		if(this.postnotify) this.postnotify(this);
		
	}
	
	changeState(){	
		this.state=!this.state;
		if(this.state) {
			this.onlabel.updateStyle("display","block");
			this.offlabel.updateStyle("display","none");
			if(this.opt.usegray) this.updateStyle("color","black");
			
			//		this.onlabel.style="display: block;";this.offlabel.style="display: none;"
		}
		else {
			this.onlabel.updateStyle("display","none");
			this.offlabel.updateStyle("display","block");
			if(this.opt.usegray) this.updateStyle("color",lightgray);
		//	if(this.notify) this.notify(this);
			//		this.onlabel.style="display: none;";this.offlabel.style="display: block;"
		}
	}
}




export {Component, ComponentFactory, TextAreaComponent, SelectComponent, DivComponent, Panel,Table,TableRow ,TableCell, TextLabel };




