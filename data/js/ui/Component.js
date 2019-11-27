

/*
 * 		Component is a class (and library) of wrappers around html components
 * 				- component can be added inside each other with addComponent
 * 				- style and properties can be changed with updateStyle() & updateProperty()
 * 				- components use local callback functions for user interaction :
 * 						- this.notify : to test if user modification is ok
 * 						- this.postnotify : when user modification has finished ok
 * 						- this.postupdate : after any this.update function call (not from user interaction)
 * 						- this.confirm : when user interaction modified the value
 * 						- this.cancel : when user cancelled modifying value
 * 						
 * 
 * */


var geid=document.getElementById.bind(document);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AbstractComponent {	//no HTML knowledge so far
	constructor(name=""){
		this.components=[];
		this.name=name;
		//	console.log("Component");
	}
	addComponent(comp){
		if(comp==undefined || comp.name==undefined){
			//problem
			console.log("problem");
			return;
		}
		this.components.push(comp);
	}
	addComponentAfter(comp,aft){
		if(comp==undefined || comp.name==undefined){console.log("addComponentAfter:: Component not defined");return;}
		var k,found=false;
		for(k in this.components){
			if(this.components[k]==aft) {found=true;break;}
		}
		if(!found) this.components.push(comp);
		else this.components.splice(k+1, 0, comp);
	}
	
	addComponentBefore(comp,bef){
		if(comp==undefined || comp.name==undefined){console.log("addComponentBefore:: Component not defined");return;}
		var k,found=false;
		for(k in this.components){
			if(this.components[k]==bef) {found=true;break;}
		}
		if(!found) this.components.push(comp);
		else this.components.splice(k, 0, comp);
	}

	removeComponent(comp){
		if(comp==undefined || comp.name==undefined) return;
		for(var k in this.components){ if(this.components[k]==comp) {this.components.splice(k, 1);return;}}
	}

	replaceComponent(comp,ncomp){
		for(var k in this.components){ if(this.components[k]==comp) {this.components[k]=ncomp;return;}}
	}

	getComponents(){return this.components;}
	// should also have
	//	- addComponent at Position 
	//	- getComponents 
	//	- getComponentNames

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

	findByName(tofind){	//recursive find component
		for(var i in this.components) {
			if(this.components[i].name==tofind) return this.components[i];
			var obj= this.components[i].findByName(tofind);
			if(obj) return obj;
		}
		return undefined;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class HTMLUtils {
	static splitAttributes(entry,opt){
		function findFirst(str,all,pos){
			var minpos=-1,mini=0;
			for(var k in all){
				var ik=str.indexOf(all[k],pos);
				if(ik>-1 && (ik<minpos || minpos==-1)) {minpos=ik;mini=k;}
			}
			return [minpos,mini];
		}
		if(!entry) return {};
		entry=entry.trim();
		var equal="=",sep=" ";
		if(opt){
			if(opt.sep) sep=opt.sep;
			if(opt.equal) equal=opt.equal;
		}
		var res={}, io=findFirst(entry,[equal,sep],0) ,ip=0;
		while(io[0]>-1){
			var k=entry.substring(ip,io[0]);
			k=k.trim();
			ip=io[0]+1;
			if(io[1]==0){	//case key=value
				var charac=sep;
				if(entry[ip]=="'" || entry[ip]=='"') {charac=entry[ip];ip++;}
				var j=entry.indexOf(charac,ip);	// should check it is not an escaped char
				if(j>-1 || (j==-1 && charac==sep)){
					if(j==-1) {res[k]=entry.substring(ip);break;}
					else res[k]=entry.substring(ip,j);
				}
				ip=j+1;
			} else if(k) res[k]=null;
			var pio1=io[1];
			io=findFirst(entry,[equal,sep],ip);
			while(io[0]==ip) {ip++;io=findFirst(entry,[equal,sep],ip);}
			if(io[0]==-1 && (entry.length-ip)>0) {
				k=entry.substring(ip,entry.length);k=k.trim();
				if(k) res[k]=null;
			}
		}		 
		return res;
	}

	static concatAttributes(atts){
		var v="";
		for(var k in atts) {v+=k+":"+atts[k]+";";}
		return v;
	}

	static htmlToElements(html) {
		var template = document.createElement('template');
		html = html.trim(); // Never return a text node of whitespace as the result
		template.innerHTML = html;
		return template.content.children;
	}
	
	static appendHTML(container,html){
		var children=HTMLUtils.htmlToElements(html);	
		while(children.length) container.appendChild(children[0]); // when appended they are removed from children pseudo-array	
	}
	static appendHTMLBefore(container,html,next){
		var children=HTMLUtils.htmlToElements(html);	
		while(children.length) {
			if(next) { var bnext=next;
				while(bnext.parentNode!=container) bnext=bnext.parentNode;
				if(bnext.parentNode==container) container.insertBefore(children[0],bnext);
				else container.appendChild(children[0]);
			}
			else container.appendChild(children[0]);
		}
	}
	

	static getTextChangeListeners(){
		var oc=	'window.objectChange(this,event)', od="window.delayEvent(this,event)";
		return {
			onchange:oc, 
			onkeydown:od,  
			onpaste:od,
			oncut:od,
			onKeyPress :od,
			onKeyUp :od,
			onblur:od
		};
	}

	static getWidthOfInput(text, prestyles, name) {
		var tmp = document.createElement("span");

		for(var s in prestyles) if(s) tmp.style[s]=prestyles[s];
		if(name && geid(name)) {
			var compstyle=getComputedStyle(geid(name));
			for(var s in compstyle) {
				if(s.startsWith("font")) tmp.style[s]= compstyle[s];
			}
		}
		tmp.innerHTML = text;				      //  tmp.innerHTML = inputEl.value.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
		document.body.appendChild(tmp);

		var theWidth = tmp.offsetWidth;			  //getBoundingClientRect().width;
		document.body.removeChild(tmp);
		return theWidth;
	}
}
///////////////////////////////

class DomComponent extends AbstractComponent { // DomComponent is the private core of Component
 
	

	findVisibleAncestor(){
		// TODO : to implement
		return null;
	}

	getContainerElement(){	// find a container to which to add
		var container;
		if(this.bottomname) container=geid(this.bottomname);
		else container=geid(this.name);
		if(!container) return this.findVisibleAncestor();
		return container;
	}

	firstRenderableChildElement(){
		for(var k in this.components) {
			var item=geid(this.components[k].name);
			if(!item) item=this.components[k].firstRenderableChildElement();
			if(item) return item;
		}
		return null;
	}
	
	lastRenderableChildElement(){
		var item;
		for(var k in this.components) {
			item=geid(this.components[k].name);
			if(!item) item=this.components[k].lastRenderableChildElement();
		}
		return item;
	}

	removeSelfDom(){
		var child=geid(this.name);
		if(!child) this.removeChildren();
		else child.parentElement.removeChild(child);
	}
	
	removeChildren(){
		for(var k in this.components) this.components[k].removeSelfDom();
	}

	getNextElement(comp){
		var next=null,prev=null;
		for(var k in this.components){if(prev==comp) {next=prev=this.components[k];break;} else prev=this.components[k];}
		var nextcomp=null;
		if(next) nextcomp=geid(next.name);
		if(next && !nextcomp) return next.firstRenderableChildElement();
		return nextcomp;
	};
	
	getPreviousElement(comp){
		var prev=null;
		for(var k in this.components){
			if(this.components[k]==comp) {break;}
			else prev=this.components[k];
		}
		var prevcomp=null;
		if(prev) prevcomp=geid(prev.name);
		if(prev && !prevcomp) return prev.lastRenderableChildElement();
		return prevcomp;
	};
	
	findParentNode(){
		for(var c in this.components){ 
			if(geid(this.components[c].name)) return geid(this.components[c].name).parentNode;
		}
		return null;
	}
	
	resetToHtml(){	// this is actually a resetToHtml() // any later modification is cancelled
		var item=geid(this.name);
		if(!item) return;
		var nhtml=this.getHtml();		 
		item.outerHTML=nhtml;
	}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Component extends DomComponent {	 // Component is the public face, DomComponent is the private side
		
	constructor(name=""){	// maybe generate an id if none given ?
		super(name);
		this.attributes={id:this.name};
	}
	
	getElement(){	//return this element or the first renderable child
		var elem=geid(this.name);
		if(!elem) return this.firstRenderableChildElement();
		else return elem;
	}

	addComponent(comp){
		super.addComponent(comp);
		var name=this.name;
		if(this.bottomname) name=this.bottomname;
		var container=geid(name);
		if(!container) return;
		var elem=geid(comp.id); // should check with children for a rendered 
		if(!elem){
			var html=comp.getHtml();
			HTMLUtils.appendHTML(container,html);
		}
		 
	}	

	addComponentAfter(comp,aft){	
		// if no aft, search for a descendant
		super.addComponentAfter(comp,aft);
		var container=this.getContainerElement();
		if(!container) return;
		var next=this.getNextElement(aft);	
		var html=comp.getHtml();
		HTMLUtils.appendHTML(container,html,next);
	}
	
	addComponentBefore(comp,bef){	
		// if no aft, search for a descendant
		super.addComponentBefore(comp,bef);
		var container=this.getContainerElement();
		if(!container) return;

		var befelement=bef.getElement();
		var html=comp.getHtml();
		if(befelement) HTMLUtils.appendHTMLBefore(container,html,befelement);
		else HTMLUtils.appendHTML(container,html);		 
	}

	addComponentAlphabeticOrder(comp){
		super.addComponentAlphabeticOrder(comp);

		var container=this.getContainerElement();
		if(!container) return;

		var next=this.getNextElement(comp);
		var html=comp.getHtml();
		HTMLUtils.appendHTML(container,html,next);		 
	}

	removeComponent(comp){
		super.removeComponent(comp);
		comp.removeSelfDom();
	}

	replaceComponent(comp,ncomp){
		var parent=geid(this.name);
		if(!parent) parent=this.findParentNode();	// if this item has no element, find parentNode of a descendant
		super.replaceComponent(comp,ncomp);
		if(!parent) return;

		var ritem=geid(comp.name);				// get the dom elementt
		var nhtml=ncomp.getHtml();
		parent.replaceChild(HTMLUtils.htmlToElement(nhtml),ritem);	

	};



	static init(){
		if(window.objectChange) return;
		window.objectChange=objectChange;
		window.delayEvent=delayEvent;
		window.subscribeObject=subscribeObject;
	}

	selfsubscribe(){
		Component.init();
		subscribeObject(this.attributes.id,this);
		this.attributes["onclick"]="window.objectChange(this);";
	}

	getPreTag(){return this.pretag;}
	getPostTag(){return this.posttag;}
	getTag(){return this.tag;}
	getTextContent(){return this.text;}


	updateProperties(propobj){for(var k in propobj) this.updateProperty(k,propobj[k]);}
	updateProperty(propname,propvalue){
		if(!propname) return;
		var item=geid(this.name);
		if(item) {
			if(propvalue)  item[propname]=propvalue;
			else item.removeAttribute(propname);
		}

		if(!this.attributes) this.attributes={};
		if(propvalue) this.attributes[propname]=propvalue;
		else delete this.attributes[propname];
	}


	removeStyles(styles){for(var k in styles)  this.updateStyle(k,"");}
	updateStylesFromString(styleString){
		var arr=styleString.split(";");
		for(var k in arr){
			var t=arr[k];
			var tt=t.split(":");
			if(tt.length==2) this.updateStyle(tt[0],tt[1]);
		}
	} 

	updateStyles(styleobj){for(var k in styleobj) this.updateStyle(k,styleobj[k]);}	
	updateStyle(subpropname, propvalue){
		if(!subpropname) return;
		if(!this.attributes.style) this.attributes.style={};
		this.attributes.style[subpropname]=propvalue;
		var item=geid(this.name);
		if(item) {
			if(propvalue) item.style[subpropname]=propvalue;
			else item.style.removeProperty(subpropname);
		}
	}

	getAttributes(){
		var attrobj={};
		Object.assign(attrobj, this.attributes);
		if(this.attributes.style){
			console.log();
		}
		if(this.style) {
			if(!attrobj.style) attrobj.style={};
			Object.assign(attrobj.style, HTMLUtils.splitAttributes(this.style,{sep:";",equal:":"}));	// <<-- styles are not like attributes : ; is the separator, : is the equality
		}
		Object.assign(attrobj, HTMLUtils.splitAttributes(this.extra));
		return attrobj;
	}

	postAttach(){
		for(var k in this.components) this.components[k].postAttach();
	}

	attachToDOM(elem,fname){
		if(!fname) fname="innerHTML";
		elem[fname]=this.getHtml();
		this.postAttach();	// notify the new state to the components
	}

	getHtml(){	
		var html="";
		if(this.debug){
			console.log();
		}
		if(this.getPreTag()) html+=this.getPreTag();
		if(this.getTag()) {
			html+="<"+this.getTag();
			var att=this.getAttributes(this);
			for(var j in att) {
				var v;
				if(typeof(att[j])=="object") v=HTMLUtils.concatAttributes(att[j]);
				else v=att[j];
				if(v!=null)	html+=" "+j+"='"+v+"'";
				else html+=" "+j;
			}
//			if(this.name) html+=" id='"+att.id+"'";
//			if(this.style) html+=" style='"+att.style+"'";	
//			if(this.extra) html+=" "+this.extra;	
			html+=">";
		}
		if(this.getTextContent()) html+=this.getTextContent();
		html+=this.getChildrenHtml();
		if(this.getTag()) html+="</"+this.getTag()+">";
		if(this.getPostTag()) html+=this.getPostTag();
		return html;
	} //return html of this component and children

	getChildrenHtml(){
		var html="";
		for(name in this.components) {
			html+=this.components[name].getHtml();
		}
		return html;
	}
}






//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////





class Panel extends Component{
	constructor(name,style,caption){
		super(name,style);
		//not compatible with style etc :(
		this.bottomname=this.name+"fieldset";
		var pretable="<div><table id='"+this.name+"' class='pure-table pure-table-bordered pure-table-striped'";
		if(style) pretable+=" style='"+style+"'";
		if(caption !=undefined) pretable+="><thead><caption>"+caption+"</caption></thead> <tbody><tr><td>";
		else pretable+="><thead><caption>"+this.name+"</caption></thead> <tbody><tr><td>";
		var posttable=" </td></tr></tbody></tbody> </table><div>";
		var prefield="<div id='"+this.name+"';" ;
		if(style) prefield+=" style='"+style+"'";
		if(caption!=undefined) prefield+="><fieldset id='"+this.bottomname+"' class='pure-fieldset'><legend>"+caption+"</legend>";
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
		this.bottomname=this.name+"tablebody"
		this.pretag="<div><table id='"+this.name+"' class='pure-table pure-table-bordered pure-table-striped'><thead><caption>"+this.name+"</caption></thead> <tbody id='"+this.bottomname+"'>";
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

class Button extends Component{
	constructor(name,text=""){
		super(name);
		this.text=text;
		this.tag="button";
		//	this.updateStyles({"cursor":"pointer"});
		this.updateProperties({"onclick":"window.objectChange(this)"});
		this.selfsubscribe();
	};
	update(text){
		this.text=text;
		document.getElementById(this.name).outerHTML = this.getHtml();
	}
	changed(){
		console.log();
		if(this.notify) this.notify();
	}
}

class TextLabel extends Component{
	constructor(name,text="", opt){
		super(name);
		this.text=text;

		if(opt && opt.nodiv) this.tag="label";
		else this.tag="div";
	};



	update(text){
		this.text=text;
		document.getElementById(this.name).outerHTML = this.getHtml();
	}
	changed(){
		console.log();
	}
}




class ClickableTextLabel extends TextLabel{
	constructor(name,val,opt){
		super(name,val,opt);
		this.updateStyles({"cursor":"pointer"});
		this.updateProperties({"onclick":"window.objectChange(this)"});
		this.selfsubscribe();
	}

	setStatusStyle(enabledstyle,disabledstyle){
		this.removeMyStyles();
		this.enabledstyle=enabledstyle;
		this.disabledstyle=disabledstyle;
		this.addMyStyles();
	}	

	setEnabled(enabled){
		if(this.enabled==enabled) return;
		this.removeMyStyles();
		this.enabled=enabled;
		this.addMyStyles();
	}

	removeMyStyles(){if(this.enabled) this.removeStyles(this.enabledstyle); else this.removeStyles(this.disabledstyle);};
	addMyStyles(){if(this.enabled) this.updateStyles(this.enabledstyle); else this.updateStyles(this.disabledstyle);};

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
		this.confirm(this.entries[val]);
		if(this.postnotify) this.postnotify(val);

	}

	update(text,newselected){
		if(!text) return;	// error : no valid value
		this.entries=text;
		if(typeof(newselected)=="undefined") this.selected=newselected; else this.selected=0;

		document.getElementById(this.name).outerHTML = this.getHtml();

		if(this.postupdate) this.postupdate(this);
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


}


class TextInputComponent extends Component{
	constructor(name,initval){
		super(name);
		if(!initval) initval="";		//we should have a more homogenous naming system throughout the components
		this.textvalue=initval;	

		this.tag="input";
		this.updateProperties({"type":"text","value":this.textvalue});
		this.updateProperties(HTMLUtils.getTextChangeListeners());

		subscribeObject(this.name,this);
		this.updateWidth();

	};	//this.style.width = ((this.value.length + 1)) + \"ch\";

	postAttach(){
		super.postAttach();
		this.updateWidth();
	}

	updateStyle(key,val){
		super.updateStyle(key,val);
		if(key!="width") this.updateWidth();
	}

	updateWidth(){
		this.width=this.getMyWidth();
		super.updateStyle("width",this.width+"px");
	}

	getMyWidth(){return HTMLUtils.getWidthOfInput(this.textvalue,this.attributes.style,this.name);}

	nextField(current){
		if(current.nextSibling) current.nextSibling.focus();
		/*	    for (var i = 0; i < current.form.elements.length; i++){
	        if (current.form.elements[i].tabIndex == current.tabIndex+1){
	            current.form.elements[i].focus();
	            if (current.form.elements[i].type == "text"){
	                current.form.elements[i].select();
	            }
	        }
	    }*/
	}

	changed(name,value,event){
		if(this.confirm && event &&  (event.type=="blur" || event.key=="Enter")) {this.confirm(value,event);this.nextField(event.target);}
		if(this.cancel && event &&  (event.key=="Escape"))	{this.cancel(value,event);this.nextField(event.target);}

		var val=geid(this.name).value;
		if(this.textvalue==val) return;

		if(this.notify) if(!this.notify(val)) return;
		this.textvalue=val;
		this.updateWidth();
		if(this.postnotify) this.postnotify(val,event);
	}

	update(text){
		if(typeof(text)=="undefined") return;	// error : no valid value
		this.textvalue=text;
		this.updateProperties({"value":this.textvalue});

		if(this.postupdate) this.postupdate(this);

//		document.getElementById(this.name).outerHTML = this.getHtml();
	}
}



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

function delayEvent(o,e){
	setTimeout(function(){objectChange(o,e);},10);
};


var objlist={};
function objectChange(o,ev) {
	var e=o.id;	
	var target=objlist[e];
	if(target) target.changed(o.id,o.value,ev,o);
	//"operations/MaxHumidity/active"
}

function subscribeObject(name, obj){
	objlist[name]=obj;
} 

var lightgray="#BBBBBB";

class SelectBinaryComponent extends BinaryComponent{
	constructor(name,offtext,ontext,startvalon,opt){
		super(name);
		if(startvalon) startvalon=1; else startvalon=0;
		this.content=new SelectComponent(name+"content",[offtext,ontext],startvalon);
		this.addComponent(this.content);
		this.tag="span";
		this.style="display:inline-block;cursor: pointer;";
		this.content.style="cursor:inherit;";
		//if(!this.content.extra) this.content.extra="";
		if(!this.extra) this.extra="";
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


class CheckBoxBinaryComponent extends BinaryComponent{
	constructor(name,inittext, val){
		super(name);
		this.text=inittext;	
		this.style="";
		this.tag="input";

		this.extra0=" onchange='objectChange(this);' type='checkbox' value='"+this.text+"' "; // onkeydown='this.style.width = ((this.value.length)) + \"ch\";delayEvent(this);' onpaste='delayEvent(this)' oncut='delayEvent(this)'  onKeyPress ='delayEvent(this)' onKeyUp ='delayEvent(this);'";
		this.extra=this.extra0;
		if(val || val=="1") this.extra+=" checked";
		this.val=val;
		subscribeObject(this.name,this);

	};

	changed(name,value,event){
		console.log("hello");
		//if(!this.val) this.update("1"); else this.update("0");	//switch val
		this.val=!this.val;
		var tval="1";
		if(!this.val) tval="0";
		if(this.confirm) {this.confirm(tval,event);	if(event) this.nextField(event.target);}
	}

	getHtml(){
		return super.getHtml();
	}
	update(val){
		this.extra=this.extra0;
		if(val=="1") {this.val=true;this.extra+=" checked";} else this.val=false;
		if(document.getElementById(this.name))	// generalize this line to higher component
			document.getElementById(this.name).outerHTML = this.getHtml();
		if(this.postupdate) this.postupdate(this);
	}
}

class LabelBinaryComponent extends BinaryComponent{
	constructor(name,offtext,ontext,starton, opt){
		if(!opt) opt={border:true,usegray:true};
		super(name);
		this.offlabel=new TextLabel(this.name+"off",offtext);
		this.onlabel=new TextLabel(this.name+"on",ontext);

		this.enabled=starton;
		if(!starton) this.onlabel.style="display: none;"
			else this.offlabel.style="display: none;"

				this.enabledstyle={"color":"black"};	
		this.disabledstyle=this.enabledstyle;	

		this.style="cursor: pointer;display:inline-block;";
		if(opt.border) {this.style+="border-style: solid;border-width: 1px;";}

		if(opt.usegray) {
			this.disabledstyle={"color":lightgray}; 
		}

		this.addMyStyles();

		this.tag="div";
		this.extra=" onclick='objectChange(this)'";
		subscribeObject(this.name,this);
		this.addComponent(this.offlabel);
		this.addComponent(this.onlabel);
		this.opt =opt;
	};

	removeMyStyles(){if(this.enabled) this.removeStyles(this.enabledstyle); else this.removeStyles(this.disabledstyle);};
	addMyStyles(){if(this.enabled) this.updateStyles(this.enabledstyle); else this.updateStyles(this.disabledstyle);};
	setStatusStyle(enabledstyle,disabledstyle){
		this.removeMyStyles();
		this.enabledstyle=enabledstyle;
		this.disabledstyle=disabledstyle;
		this.addMyStyles();
	}	


	changed(name,value){
		console.log("Changed ! "+name+" "+value);
		if(this.notify && !this.notify(this)) return;
		this.changeState();
		if(this.postnotify) this.postnotify(this);

	}

	changeState(){			
		this.removeMyStyles();
		this.enabled=!this.enabled;
		if(this.enabled){
			this.onlabel.updateStyle("display","block");
			this.offlabel.updateStyle("display","none");
		} else {
			this.onlabel.updateStyle("display","none");
			this.offlabel.updateStyle("display","block");
		}
		this.addMyStyles();

	}

}




export {Component, ComponentFactory, TextInputComponent, TextAreaComponent, Button,
	SelectComponent, DivComponent, Panel,Table,TableRow ,TableCell, TextLabel,
	LabelBinaryComponent, CheckBoxBinaryComponent, SelectBinaryComponent, ClickableTextLabel};




