
import {EventEmitter} from "../EventEmitter.js";

import {Component, ComponentFactory, TextInputComponent, SelectComponent, DivComponent, Panel,Table,TableRow ,TableCell, TextLabel, LabelBinaryComponent, CheckBoxBinaryComponent, SelectBinaryComponent, ClickableTextLabel} from "./Component.js";

import {LogGraphGroup } from "./LogDygraph.js";

import {GenericNodeView} from "./NodeView.js";

var geid=document.getElementById.bind(document);
var lightgray="#BBBBBB";


 


class UserView extends EventEmitter {
	constructor(){
		super();
		console.log("UserView");
		this.panel=new Panel();
		this.panel.addPanel("nodes","display:inline-block;");	// initialize with nodes and operations // or should we order the groups alphabetically?
		this.panel.addPanel("operations","display:inline-block;");
	}

	removeNode(nodeview,name,group,subgroup){
		console.log(name);
		if(this.panel.hasComponent(group)){
			var grouppanel=this.panel.getComponent(group);
			if(grouppanel.hasComponent(subgroup)) {
				var subgrouppanel = grouppanel.getComponent(subgroup);
				subgrouppanel.removeComponent(nodeview.getComponent());
				if(!subgrouppanel.components.length) grouppanel.removeComponent(subgrouppanel);
				if(nodeview.selfRemove) nodeview.selfRemove();
				//if empty subgroup, remove subgroup as well
			}
		}
		
	}

	createNode(name,group,subgroup,data){
		var grouppanel,subgrouppanel;
		if(this.panel.hasComponent(group)) {
			grouppanel=this.panel.getComponent(group);
			if(grouppanel.hasComponent(subgroup)) 
				subgrouppanel=grouppanel.getComponent(subgroup);
			else {
				subgrouppanel = new Table(subgroup);
				grouppanel.addComponent(subgrouppanel);	
			}
		} else {
			grouppanel = this.panel.addPanel(group,"display:inline-block;");
			subgrouppanel = new Table(subgroup);
			grouppanel.addComponent(subgrouppanel);
		}

		var nview= new GenericNodeView(group+'/'+name,data);
		subgrouppanel.addComponentAlphabeticOrder(nview.getComponent());	//add it alphabetically sorted

		return nview;
	};


	updateServerInfo(serverinfo){
		if(!this.serverview) {
			this.serverview=new ServerView(serverinfo);
			//  this.panel.addComponent(this.serverview.getComponent());
			var serverhtml=this.serverview.getComponent().getHtml();	
			geid('serverstatus').innerHTML=serverhtml;	//should not be added like that
			this.serverview.init();	
		} else this.serverview.update(serverinfo);
	}	


	updateLogView(data,labels,nodes){
		if(!data || data.length==0 || data[0].length==0) {console.log("Log data table received from /log/ empty.");return;}
		var tooshort=true;
		if(data.length>1)
			for(var j in data) if(data[j].length>1) {tooshort=false;break;}
		if(tooshort) return;

		if(!this.logview) {
			this.logview= new LogGraphicView(data,labels,nodes);	
			//this.panel.addComponent(this.logview.getComponent());
			var loghtml=this.logview.getComponent().getHtml();	
			geid('loginterface').innerHTML=loghtml;	//should not be added like that
			this.logview.init();	// is it really useful to have a separate init ? yes, we need to add the component to the ui before init
		} else this.logview.updateGraph(data,labels,nodes);		
	}

	
	init(){	 
		this.panel.attachToDOM(geid('userinterface'));
	}

	
	notifyConfirm(event){
		console.log(event);
		// update value and revert to black
		
	}
	notifyCancel(event){
		console.log(event);
	}
	
	update(){

	}
};

/*
class Formatter {
	constructor(){
		/*this.symbols={"greaterorequal":">=","less":"<"};
		this.symbolsnamepattern=["operator"];
		this.boolnamepattern=["active","cyc","conditionmet"];
		this.timestampnamepattern=["ts"];* /
		var ggd=globalThis.globaldefs.data;
		this.symbols=ggd.valueformat.symbols;
		this.symbolsnamepattern=ggd.fieldformat.symbols;
		this.boolnamepattern=ggd.fieldformat.boolnamepattern;
		this.timestampnamepattern=ggd.fieldformat.timestampnamepattern;

	};

	isSymbol(val){		
		for(var s in this.symbols) 
			if(val.indexOf(s)>-1) return true;
		return false;
	}
	replaceSymbols(val){		
		for(var s in this.symbols) 
			if(val.indexOf(s)>-1) val=val.replaceAll(s,this.symbols[s]);
		return val;
	}

	formatDetect(value,varname){
		if(!value) return "";
		if((value[0]=='/' || value.substring(0,2)=="./")) return "link";

		if(Number(value) > 946684800) return "ts";//21st century
		for(var s in this.timestampnamepattern) if(varname.indexOf(this.timestampnamepattern[s])>-1) return "ts";

		for(var s in this.symbols) if(value.indexOf(s)>-1) return "sym";
		for(var s in this.symbolsnamepattern) if(varname.indexOf(this.symbolsnamepattern[s])>-1) return "sym";

		for(var s in this.boolnamepattern) if(varname.indexOf(this.boolnamepattern[s])>-1) return "bool";
		if(!value) return "";
	}

	syntaxDetect(value,varname){
		if(!value) return "";
		if(value.indexOf("{")>-1 && value.indexOf("}")>-1 ) return "enum"; 	//symbol
	}

	getEnumCases(value){
		var ar=[];
		var index=value.indexOf("{");
		if(index<0) return ar;
		ar.push(value.substring(0,index));
		var pindex=value.indexOf(",",index+1);
		while(pindex>-1) {
			ar.push(value.substring(index+1,pindex));
			index=pindex;
			pindex=value.indexOf(",",index+1);
		}
		pindex=value.indexOf("}",index+1);
		if(index<0) return ar;
		ar.push(value.substring(index+1,pindex));
		return ar;
	}

	format(value,varname,data){
		var format=this.formatDetect(value,varname);
		if(format=="ts" && value>0) return new Date(Number(value)).toLocaleString(); 
		if(format=="sym") return this.replaceSymbols(value);
		return value;
	}

	getStyle(value,varname){
		var format=this.formatDetect(value,varname);
		if(format=="ts") return {"font-size":"70%"};
		if(format=="link") return {"font-size":"70%","text-decoration": "underline"};
		return "";
	}

	getUnit(varname, data){
		var concatab=	{"val":"unit","min":"unit"};		// fields that must be concatened in value
		// maybe later we should follow link to take the unit of the target
		if(data && 	(varname in concatab)){
			if( (concatab[varname] in data)) return data[concatab[varname]];
		}
		return null;
//		if(!data) return data; return data["unit"];
	}
	
	formatCompatible(value,format){
		if(format=="bool"){
			var v=Number.parseFloat(value);
			if(v==0 || v==1) return true;			
		}
		return false;
	}
	
	getCompatibleComponents(value,varname){
		var compnames=[],boolformat=false, booltab=["CheckBoxBinaryComponent","LabelBinaryComponent","SelectBinaryComponent"];
		 
		if((this.formatDetect(value,varname)=="bool")) {	// by default bool
			compnames=compnames.concat(booltab);
			boolformat=true;
		} else if(this.syntaxDetect(value,varname)=="enum") {
			compnames.push("SelectComponent");
		} 
		compnames.push("TextInputComponent");
		if(!boolformat && this.formatCompatible(value,"bool")) compnames=compnames.concat(booltab); // optionally bool
			
		return compnames;
			 
	}
	
	createComponent(typename,id,value){
		switch(typename){
		case "LabelBinaryComponent" : 
			return new LabelBinaryComponent (id,"Off","On",value=="1");
		case "SelectBinaryComponent" : 
			return new SelectBinaryComponent (id,"Off","On",value=="1");
		case "CheckBoxBinaryComponent" : 
			return new CheckBoxBinaryComponent (id,"",value=="1");
		case "SelectComponent":
			return new SelectComponent (id, this.getEnumCases(value));
		case "TextInputComponent":
			return new TextInputComponent(id,value);
		}
		return new TextInputComponent(id,value);
		 
	}
}


class ValueView extends DivComponent {
	constructor(nodename,varname,value, data,emitter){	
		super(nodename+'/'+varname+"#value");
		var	formatter=new Formatter();
		value=formatter.format(value,varname,data);
		if(varname=="operator") {
			console.log();
		}
		var st=formatter.getStyle(value,varname);
		var valuecomp,mevalueview=this;
		
		// get a list of favorite/compatible formats for this var
		
		var favorites=formatter.getCompatibleComponents(value,varname);
		var currentfav=0;
		// create most favorite
		valuecomp=formatter.createComponent(favorites[currentfav],nodename+'/'+varname,value);
		valuecomp.initval=value;
		valuecomp.postnotify=function(){
			// become gray if modified until enter or focusout, then it is saved to the server, become orange, until confirmed
			valuecomp.updateStyle("color","lightgray");
			console.log(valuecomp.name);
			
		};
		valuecomp.confirm=function(){
			if(valuecomp.textvalue==valuecomp.initval) {valuecomp.updateStyle("color","black");return;} // no change
			// unless it did not change, it should become blue and be sent to the server
			valuecomp.updateStyle("color","blue");
			var obj={};
			obj[valuecomp.name]=valuecomp.textvalue;
			emitter.emit("updateValue",obj);
		}
		
		
		this.addComponent(valuecomp);
		var unit=formatter.getUnit(varname,data)
		if(unit) {
			var unitcomp=new TextLabel(nodename+varname+"label",unit,{nodiv:true});
			unitcomp.updateStyles({"font-size":"80%","color":"gray"});
			this.addComponent(unitcomp);
		}
		if(favorites.length>1){
			// add a triangle with component switch on change
			var triangle=new ClickableTextLabel(nodename+varname+"labelminus"," ^ ",{nodiv:true});	
			triangle.updateStyles({"font-size":"60%", "color":"gray", "cursor": "pointer"});
			//	triangle.setStatusStyle(onstyle,offstyle);
			triangle.setEnabled(true);
			triangle.changed=function(){
				currentfav++;
				if(currentfav>=favorites.length) currentfav=0;
				var nvaluecomp=formatter.createComponent(favorites[currentfav],nodename,varname,value);
				mevalueview.replaceComponent(valuecomp,nvaluecomp);
			}
			this.addComponent(triangle);
		}
		

		if(st) valuecomp.updateStyles(st);

	}
}

class PlusMinusElements {
	constructor(parentcomp,nodename,varname,data){
		var onstyle={"color":"gray", "cursor": "pointer"};
		var offstyle={"color":"lightgray", "cursor": ""};
		var cells=Object.keys(data[varname]).length;
		
		var minus=new ClickableTextLabel(nodename+varname+"labelminus"," - ",{nodiv:true});	
		minus.updateStyles({"font-size":"70%"});
		minus.setStatusStyle(onstyle,offstyle);
		if(cells>0) minus.setEnabled(true);
		else minus.setEnabled(false);
		minus.changed=function(){
			console.log();
			if(mevarview.lines<1) return ;
			var comps=mevarview.getComponents();
			if(!comps) return;
			var comp=comps[comps.length-1];
			mevarview.removeComponent(comp);
			mevarview.lines--;
			if(mevarview.lines<1) minus.setEnabled(false);				 
		}
		parentcomp.addComponent(minus);
		var mevarview=this;
		
		var plus=new ClickableTextLabel(nodename+varname+"labelplus"," + ",{nodiv:true});
		plus.updateStyles({"font-size":"70%"});
		plus.setStatusStyle(onstyle,offstyle);
		plus.setEnabled(true);
		plus.changed=function(){
			var value="";
			var path=varname+'/'+mevarview.lines;
			var varview=new ValueView(nodename,path,value, data);
			mevarview.addComponent(varview);	// this so far is useless as it workd for the start, 
													// let's make it self update the html tree  
			mevarview.lines++;		 
		}			
		parentcomp.addComponent(plus);
	}
	
}



class VarView extends Component {
	constructor(nodename,varname,data,emitter){
		super(nodename+varname+"varview");
		this.lines=1;

		var valuelabel=new TextLabel(nodename+varname+"label",varname+":",{nodiv:true});
		valuelabel.style="font-size:70%;color:darkgray;";
 		this.addComponent(valuelabel);

		if(typeof(data[varname])=="object") {
			new PlusMinusElements(this,nodename,varname,data);
			for(var subvar in data[varname]) {
				var value=data[varname][subvar];
				var path=varname+'/'+subvar;
				var varview=new ValueView(nodename,path,value, data,emitter);
				this.addComponent(varview);this.lines++;
			}	
		} else {
			var varview=new ValueView(nodename,varname,data[varname],data,emitter);
			this.addComponent(varview);this.lines++;
		}


	}
}




class NodeFormatter{	
	static getOrder(ddata){
		var ggd=globalThis.globaldefs.data;
		var byclassfullorder=ggd.byclassfullorder;
		var byfieldorder=ggd.byfieldorder;
		var seq=byfieldorder.seq;
		var classfield=ggd.classfield;
		var typefield=ggd.typefield;
		
		var before=[],after=[],superafter=[], final=[];

		console.log(ddata);

		function finditin(item,tab){
			for(var k in tab) {
				if(tab[k]==item) {return true;}
			}
			return false;
		}
		function findinseq(v){
			for(var k in seq) {if(seq[k]==v && k in ddata)	return true;}	// if current ddata varname is a value in seq with a predecessor keyname in ddata too
			return false;
		}
		var type;
		if(classfield in ddata) type=ddata[classfield];
		if(typefield in ddata) type=ddata[typefield];
		var bcfo=byclassfullorder[type];
		if(type && bcfo){	
			for(var k in bcfo.main) if(bcfo.main[k] in ddata) final.push(bcfo.main[k]);
			for(var k in bcfo.detail) if(bcfo.detail[k] in ddata) after.push(bcfo.detail[k]);
		}

		for(var it in ddata) {
			if(finditin(it,final) || finditin(it,after)) continue;
			if(finditin(it,byfieldorder.overview)) {before.push(it);continue;}
			if(ddata[it].substring && ddata[it].substring(0,2)=="./") {
				after.push(it);
				continue;}
			if(finditin(it,byfieldorder.detail)) {after.push(it);continue;}
			if(finditin(it,byfieldorder.superdetail)) {superafter.push(it);continue;}

			if(!findinseq(it)) final.push(it);						// if field name not known, add it to main list
			if(it in seq && seq[it] in ddata) final.push(seq[it]);
		}
//		before=before.concat(final);
//		before=before.concat(after);
//		before=before.concat(superafter);
		return [before.concat(final),after,superafter];
	};
};


class ArrowButtons extends Component {
	constructor(name,data){
		super();

		this.order=this.getOrder(data);	

		this.arrowless=new TextLabel(name+"arrowless","&#9665;",{nodiv:true});
		this.arrowmore=new TextLabel(name+"arrowmore","&#9655;\t",{nodiv:true});

		var mearrow=this;
		mearrow.currentlevel=0;
		this.arrowless.changed=(function(){
			console.log("less");
			if(mearrow.currentlevel>0) {
				mearrow.currentlevel--;
				while(mearrow.order[mearrow.currentlevel].length==0 && mearrow.currentlevel>0) mearrow.currentlevel--;
			}
			else return;
			for(var k in mearrow.varviews){
				var n=mearrow.varviews[k] ;
				function identical(e){return e==k;}
				if(mearrow.currentlevel<1 && mearrow.order[1].find(identical)) n.updateStyle("display","none");// must display now
				if(mearrow.currentlevel<2 && mearrow.order[2].find(identical)) n.updateStyle("display","none");// must display now
				if(mearrow.currentlevel==0) mearrow.arrowless.updateStyle("color","lightgray");	//disable cursor too
				else mearrow.arrowless.updateStyle("color","");
				if(mearrow.currentlevel==2) mearrow.arrowmore.updateStyle("color","lightgray");
				else mearrow.arrowmore.updateStyle("color","");
			}

		}).bind(this.arrowless);
		this.arrowmore.changed=(function(){
			console.log("more"+mearrow);
			if(mearrow.currentlevel<2) {
				mearrow.currentlevel++;
				while(mearrow.order[mearrow.currentlevel].length==0 && mearrow.currentlevel<2) mearrow.currentlevel++;
			}
			else return;
			for(var k in mearrow.varviews){
				var n=mearrow.varviews[k] ;
				function identical(e){return e==k;}
				if(mearrow.currentlevel>=1 && mearrow.order[1].find(identical)) n.updateStyle("display","");// must display now
				if(mearrow.currentlevel>=2 && mearrow.order[2].find(identical)) n.updateStyle("display","");// must display now
				if(mearrow.currentlevel==2) mearrow.arrowmore.updateStyle("color","lightgray");
				else mearrow.arrowmore.updateStyle("color","");
				if(mearrow.currentlevel==0) mearrow.arrowless.updateStyle("color","lightgray");	//disable cursor too
				else mearrow.arrowless.updateStyle("color","");
			}

		}).bind(this.arrowmore);

		this.arrowmore.selfsubscribe();
		this.arrowmore.style="cursor: pointer;";
		this.arrowmore.extra=" onclick='window.objectChange(this)'";
		if(this.currentlevel==2) this.arrowmore.style+="color:lightgray;";	//disable cursor too

		this.arrowless.selfsubscribe();
		this.arrowless.style="cursor: pointer;"; 
		this.arrowless.extra=" onclick='window.objectChange(this)'";
		if(this.currentlevel==0) this.arrowless.style+="color:lightgray;";	//disable cursor too

		this.addComponent(this.arrowless);
		this.addComponent(this.arrowmore);
	}

	getOrder(data){
		this.order=NodeFormatter.getOrder(data);
		return this.order;}

	addVarView(name,varview){
		if(!varview) return;
		if(!this.varviews) this.varviews={};
		this.varviews[name]=varview;
	}
}

///////////////
class GenericNodeView extends EventEmitter {
	constructor(name,data){
		super();
		this.name=name;
		this.component=new Component(name);
		// add text label for name
		// add interactive text field/radio button for value with optional title 'value
		var row=new TableRow(name+"row");
		this.component.addComponent(row);		
		var cell=new TableCell(name+"cellname");
		//cell.style="padding:5px";
		row.addComponent(cell);	
		var namelabel=new TextLabel(name+"name",name.split("/").pop(),{nodiv:true});
		var arrows=new ArrowButtons(name,data);
		cell.addComponent(arrows);
		cell.addComponent(namelabel);
		var selected=arrows.getOrder(data);

		var ordered=selected[0];
		ordered=ordered.concat(selected[1]);
		ordered=ordered.concat(selected[2]);
		this.varviews=[];
		for(var i in ordered){
			var t=ordered[i];
			var cell2=new TableCell(name+"cell"+t);
			row.addComponent(cell2);
			var varview=new VarView(name,t,data, this);			
			 	
			arrows.addVarView(t, cell2);
			cell2.style="text-align:center;padding:";
			if(varview.lines>2) cell2.style+="0px 3px 0px 3px;";
			else cell2.style+="3px 5px 3px 5px;";
			if(!selected[0].find(function(e){return t==e;})) cell2.style+="display:none;";

			cell2.addComponent(varview);
			this.varviews.push(varview);
		}
	}



	update(data){
		
		
		for(var k in this.varviews) {
			this.varviews[k].update(data);
		}
		
		
		
		
		/*
		var val=this.component.findByName(this.name+"value");
		if(val){
			var valtext=""+data.val;
			if(data.unit) valtext+=data.unit;
			if(val.text!=valtext) {
				val.update(valtext);
				if(data.val) this.alarmview.update(data.val);
			}
		}
		var minmaxlabel=this.component.findByName(this.name+"minmax");
		if(minmaxlabel)		{
			var unit;
			if(data.unit) unit=data.unit;
			var minmaxtext="["+data["minval"]+unit+", "+data["maxval"]+unit+"]";
			if(minmaxlabel.text!=valtext) minmaxlabel.update(minmaxtext);
		}
* /
	};

	getComponent(){
		return this.component;
	}
}

///////////////
class FixedNodeView extends EventEmitter{
	constructor(name,data){
		super();
		this.name=name;
		this.component=new Component(name);
		// add text label for name
		// add interactive text field/radio button for value with optional title 'value
		var row=new TableRow(name+"row");
		this.component.addComponent(row);

		var cell=new TableCell(name+"cellname");
		row.addComponent(cell);	
		var namelabel=new TextLabel(name+"name",name);
		cell.addComponent(namelabel);

		var cell2=new TableCell(name+"cellvalue");
		cell2.style="text-align:center";
		row.addComponent(cell2);
		var unit=data.unit;
		if(unit==undefined) unit="";
		var valuelabel=new TextLabel(name+"value",""+data.val+""+unit);
		cell2.addComponent(valuelabel);

		var cell3=new TableCell(name+"cellminmax");
		cell3.style="text-align:center";
		row.addComponent(cell3);
		var minmaxlabel=new TextLabel(name+"minmax","["+data["minval"]+unit+", "+data["maxval"]+unit+"]");
		minmaxlabel.fontsize=0.7;
		cell3.addComponent(minmaxlabel);

		var cell4=new TableCell(name+"alarm");
		cell4.style="text-align:center";
		row.addComponent(cell4);
		this.alarmview=new AlarmView(name,data.val);
		cell4.addComponent(this.alarmview.component);
	}

	update(data){
		var val=this.component.findByName(this.name+"value");
		if(val){
			var valtext=""+data.val;
			if(data.unit) valtext+=data.unit;
			if(val.text!=valtext) {
				val.update(valtext);
				if(data.val) this.alarmview.update(data.val);
			}
		}
		var minmaxlabel=this.component.findByName(this.name+"minmax");
		if(minmaxlabel)		{
			var unit;
			if(data.unit) unit=data.unit;
			var minmaxtext="["+data["minval"]+unit+", "+data["maxval"]+unit+"]";
			if(minmaxlabel.text!=valtext) minmaxlabel.update(minmaxtext);
		}

	};

	getComponent(){
		return this.component;
	}
}
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
function removeA(arr) {
	var what, a = arguments, L = a.length, ax;
	while (L > 1 && arr.length) {
		what = a[--L];
		while ((ax= arr.indexOf(what)) !== -1) {
			arr.splice(ax, 1);
		}
	}
	return arr;
}



var lightred="#FFDDDD";

function getNumber(text){
	if(typeof(text)=="undefined") return Number.NaN;
	while(text.search(" ")>=0) text=text.replace(" ", "");
	if((text.length || isNaN(text))) return Number.NaN;
	return Number(text);
}

class AlarmView {
	constructor(name,valtext){
		this.name=name+"alarm";
		this.value=getNumber(valtext);
		this.component=new DivComponent(this.name);
		/*
		var namelabel=new TextLabel(this.name+"name","Alarm:");

		this.component.addComponent(namelabel);
		 */
		this.component.style="width:100;vertical-align: text-top;border-style: solid;border-width: 1px;border-color:"+lightgray+";padding:2px;";

		var activecomp=ComponentFactory.create(this.name+"active","BinaryComponent","labels","\uD83D\uDD14","\uD83D\uDD14",{usegray:true} );
		activecomp.style+="font-size:120%; font-weight: bold;vertical-align:inherit;";
		this.component.addComponent(activecomp);
		if(!activecomp.posttag)activecomp.posttag="";
		activecomp.posttag+="&nbsp;&nbsp;&nbsp;";

		var textcomp=new TextInputComponent(this.name+"text","",1.5);

		/*	var spacecomp=new TextLabel(this.name+"space","&nbsp;");
		spacecomp.style="display:inline-block;";
		this.component.addComponent(spacecomp);
		 */


		var comparecomp=ComponentFactory.create(this.name+"compare","BinaryComponent","select",">","\u2264");
		comparecomp.style+="font-size:120%; font-weight: bold;vertical-align:inherit;";
		this.component.addComponent(comparecomp);
		if(!comparecomp.posttag)comparecomp.posttag="";
		comparecomp.posttag+="&nbsp;"

			//	var textcomp=new TextAreaComponent(this.name+"text","  ",1.5);
			textcomp.style+="font-size:100%;vertical-align:inherit;";
		this.component.addComponent(textcomp);
		if(!textcomp.posttag) textcomp.posttag="";
		textcomp.posttag+="&nbsp;&nbsp;"

			//var loopcomp=ComponentFactory.create(this.name+"loop","BinaryComponent","labels","\u21e5","\u2940");
			var loopcomp=ComponentFactory.create(this.name+"loop","BinaryComponent","labels","\u27f6 ","\u27f2 ",{usegray:false,border:true});
		loopcomp.style+="font-size:60%; font-weight: bold;vertical-align:inherit;padding:2px 3px 0px 2px;";
		if(!loopcomp.posttag)loopcomp.posttag="";
		loopcomp.posttag+="&nbsp;"

			var filecomp=new SelectComponent(this.name+"file",["bell.mp3","foghorn.mp3"]);
		filecomp.style+="font-size:55%;padding:2px;vertical-align:inherit;";
		if(!filecomp.posttag)filecomp.posttag="";
		filecomp.posttag+="&nbsp;"
			this.component.addComponent(filecomp);
		this.component.addComponent(loopcomp);

		function flashred(text){
			var ntext=getNumber(text);
			if(isNaN(ntext)) {
				textcomp.updateStyle("backgroundColor",lightred);
				setTimeout(function(){
					textcomp.updateStyle("backgroundColor","white");
				},2000);
				return false;}

			return true;
		};

		activecomp.notify=function(text){
			if(flashred(textcomp.text)) {this.switchAlarm();return true;}
			return false;
		}.bind(this);

		textcomp.notify=function(text){
			if(flashred(text)) {textcomp.text=text;this.updateAlarm();return true;}
			return false;
		}.bind(this);

		loopcomp.postnotify=function(comp){
			if(!this.runningstate) return;
//			var item=geid(this.name+"audio");
			var item=AudioFiles.sources[this.name+"audio"];
			if(comp.state ==1) {// loop is on
				if(item) AudioFiles.setLoop(this.name+"audio",true);
				else AudioFiles.startSound(this.name+"audio",this.filecomp.entries[this.filecomp.selected],this.loopcomp.state);
				//playSound(this.filecomp.entries[this.filecomp.selected], this.name+"audio", true);
			} else {
				if(item) AudioFiles.setLoop(this.name+"audio",false);
			}
		}.bind(this);

		filecomp.postnotify=function(text){
			if(!this.runningstate) return ;
			text=this.filecomp.entries[this.filecomp.selected];
			var item=AudioFiles.sources[this.name+"audio"];
			if(this.runningstate){
				if(item && item[0].url!=text) AudioFiles.stopSound(this.name+"audio");
				AudioFiles.startSound(this.name+"audio",this.filecomp.entries[this.filecomp.selected],this.loopcomp.state);
			}
		}.bind(this);

		comparecomp.postnotify=function(text){
			this.updateAlarm();
		}.bind(this);

		this.activecomp=activecomp;
		this.comparecomp=comparecomp;
		this.textcomp=textcomp;
		this.loopcomp=loopcomp;
		this.filecomp=filecomp;
		this.activestate=false;
		this.runningstate=false;

	};


	switchAlarm(){
		console.log("switchAlarm");
		if(!this.activestate) {
			// test
			this.activestate=true;
			this.updateAlarm();

			//start audio file
			//glow red					
		} else {
			this.activestate=false;
			this.updateAlarm();
		}
	}

	updateAlarm(){
		if(this.activestate) {//active
			var b=this.testAlarm();
			if(b && !this.runningstate) this.startAlarm(); //true and not running -> start
			if(!b && this.runningstate) this.stopAlarm(); //false and running -> stop
		} else if(this.runningstate) this.stopAlarm(); // stop without test
	}

	update(valtext){
		this.value=getNumber(valtext);
		this.updateAlarm();
	}

	testAlarm(){
		var ncomp=getNumber(this.textcomp.text);
		var comp=">";
		if(this.comparecomp.state==1) comp="<=";
		if(comp==">" && this.value>ncomp) return true;
		if(comp=="<=" && this.value<=ncomp) return true;
		return false;
	}

	startAlarm(){
		this.activecomp.opt.usegray=false;
		this.activecomp.updateStyle("color","#FFAAAA");
		this.runningstate=true;
		AudioFiles.startSound(this.name+"audio",this.filecomp.entries[this.filecomp.selected],this.loopcomp.state);
		//playSound(this.filecomp.entries[this.filecomp.selected],this.name+"audio",this.loopcomp.state);

	}
	stopAlarm(){
		this.activecomp.opt.usegray=true;
		this.activecomp.updateStyle("color","black");
		this.runningstate=false;
		AudioFiles.stopSound(this.name+"audio");
	}

};

var AudioFiles={
		databuffers:{},
		sources:{},
		audioContext : new AudioContext(),
		loadAudioFile:function(url,onloaded){
			var request = new XMLHttpRequest();
			request.open('GET', url, true);
			request.responseType = 'arraybuffer';
			request.onload = function() {
				var audioData = request.response;
				this.audioContext.decodeAudioData(audioData, function(buffer) {	databuffer=buffer;onloaded(buffer);},	
						function(e){"Error with decoding audio data" + e.err});
			}.bind(this);
			request.send();
		},	
		startFromBuffer:function(key,url,loop=false){
			if(!this.databuffers[url]) return false;
			var source = this.audioContext.createBufferSource();
			source.buffer = this.databuffers[url];
			source.connect(this.audioContext.destination);
			source.loop = loop;
			source.onended=function(){
				source.disconnect();
				if(this.sources[key]) {
					removeA(this.sources[key],source);
					if(this.sources[key].length==0) delete this.sources[key];
				}
			}.bind(this);
			source.start(0);
			source.url=url;
			if(!this.sources[key]) this.sources[key]=[];
			this.sources[key].push(source);
			//setTimeout(function(){source.stop();},1000);
			return true;
		},
		stopSound:function(key){// stop all sources with this key
			if(!this.sources[key]) return;
			for(var s in this.sources[key]) {
				this.sources[key][s].stop();
				removeA(this.sources[key],this.sources[key][s]);

			}
			if(this.sources[key].length==0) delete this.sources[key];

		},
		setLoop:function(key,loop){	//set loop for all sources
			for(var s in this.sources[key]) this.sources[key][s].loop=loop;
		},
		startSound:function(key,url,loop){
			if(!this.databuffers[url]) this.loadAudioFile(url,function(buffer){this.databuffers[url]=buffer;this.startFromBuffer(key,url,loop);}.bind(this));
			else this.startFromBuffer(key,url,loop);
		}
		/* demo
		AudioFiles.startSound("alarm1","bell.mp3",false);
		setTimeout(function(){AudioFiles.setLoop("alarm1",true);},2000);
		setTimeout(function(){AudioFiles.stopSound("alarm1")},7000);
		 */
};
 

///////////////
///////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ServerView {
	constructor(serverinfo){
		this.name="serverview";
		this.serverinfo=serverinfo;
		this.component= new Panel("Status","font-size:85%;display:inline-block;");

		var table = new Table("");

		this.component.addComponent(table);
		var row=new TableRow(this.name+"row");
		var cell=new TableCell(this.name+"cell");

		table.addComponent(row);
		row.addComponent(cell);
		this.serverstatuslabel=new TextLabel(this.name+"label",serverinfo.status);

		this.serverreceivedlabel=new TextLabel(this.name+"label2",this.getReceivedMessage(serverinfo));
		if(serverinfo.status=="online") {
			this.serverstatuslabel.style="color:green;font-weight:bold;";
			this.serverstatuslabel.fontsize=1.2;
		}
		cell.addComponent(this.serverstatuslabel);
		cell.addComponent(this.serverreceivedlabel);


		if(serverinfo.lastresponsetime) {
			var label1=new TextLabel(this.name+"lastresponsetime1","last response ");
			this.lastresponsetimelabel=new TextLabel(this.name+"lastresponsetime2",this.formatDate(serverinfo.lastresponsetime));
			this.lastresponsetimelabel.fontsize=0.8;
			var cell2=new TableCell(this.name+"cell3");
			cell2.addComponent(label1);
			cell2.addComponent(this.lastresponsetimelabel);
			row.addComponent(cell2);
		}
		if(serverinfo.boottime) {
			var label1=new TextLabel(this.name+"boottime1","started ");
			this.boottimelabel=new TextLabel(this.name+"boottime2",new Date(Number(serverinfo.boottime)).toLocaleString());
			this.boottimelabel.fontsize=0.8;
			var cell2=new TableCell(this.name+"cell2");
			cell2.addComponent(label1);
			cell2.addComponent(this.boottimelabel);
			row.addComponent(cell2);
		}

		//this.component.addComponent(label);
	}

	getReceivedMessage(serverinfo){
		var str="no data/log received";
		var datareceived=false,logreceived=false;
		if(serverinfo.load || serverinfo.reload) datareceived=true;
		if(serverinfo.logload || serverinfo.logreload) logreceived=true;
		if(datareceived && logreceived) str="data&log received";
		else if(datareceived) str= "data received/no log";
		else if(logreceived) str= "no data/log received";
		return str;
	}

	formatDate(t){return new Date(Number(t)).toLocaleString();};

	init(){};
	getComponent(){return this.component;}

	update(serverinfo){
		if(this.lastresponsetimelabel && this.lastresponsetimelabel.text!=this.formatDate(serverinfo.lastresponsetime)) this.lastresponsetimelabel.update(this.formatDate(serverinfo.lastresponsetime));
		if(this.boottimelabel && this.boottimelabel.text!=this.formatDate(serverinfo.boottime)) {
			this.boottimelabel.update(this.formatDate(serverinfo.boottime));
			this.boottimelabel.updateStyle("color","red");
		}
		if(this.serverstatuslabel && this.serverstatuslabel.text!=serverinfo.status) {
			this.serverstatuslabel.update(serverinfo.status);
			if(serverinfo.status=="online") {
				this.serverstatuslabel.updateStyle("color","green");
				this.lastresponsetimelabel.updateStyle("color","black");
			}
			else {
				this.serverstatuslabel.updateStyle("color","red");
				//	this.serverstatuslabel.updateStyle("font-weight","bold");
				this.lastresponsetimelabel.updateStyle("color","red");
			}
		}
		if(this.serverreceivedlabel && this.serverreceivedlabel.text!=this.getReceivedMessage(serverinfo)) this.serverreceivedlabel.update(this.getReceivedMessage(serverinfo))


	};
};

///////////////
///////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LogGraphicView {
	constructor(data,labels,nodes){
		this.name="graphiclog";
		this.rawdata=data;
		this.labels=labels;
		this.nodes=nodes;
//		this.component= new Panel(this.name+"container","","log ("+this.countPoints(data)+" time point)");
		this.component= new DivComponent(this.name+"container","","log ("+this.countPoints(data)+" time point)");
		this.graphgroup=new LogGraphGroup(this.name,this.component,nodes);
	//	this.graphgroup.init(this.rawdata,this.labels,this.nodes);
	}

	countPoints(data){	//count lines starting with a number
		return data.length;
		/*		var c=0;
		for(var l in data){
			if(typeof(data[l][0])=="number") c++;
		}
		return c;*/
	}
 
	init(){
//		this.addGraph(this.rawdata);
//		}addGraph(data){ 
		this.graphgroup.init(this.rawdata,this.labels,this.nodes);
	}
 


	updateGraph(data,labels,nodes){
		this.graphgroup.update(data,labels,nodes);	// he is not supposed to change data object !!!
		//.innerHTML=this.component.getChildrenHtml();	//update visible result
	}

	getComponent(){return this.component;}
};





///////////////
class LogTableView {
	constructor(data){
		this.name="log";
		this.rawdata=data;
		this.component= new Table(this.name);

		this.addRows(data);
	}
	addRows(data){
		for(var channel in data){
			this.addRow("logchannelname"+channel,channel," ",1); var i=0;
			var channeldata=data[channel].slice(0).reverse();
			for(var index in channeldata){
				var entry=channeldata[index];
				var d=new Date(Number(entry[0]));
				var datetext=d.toString().substring(4,24);

				this.addRow("logdata"+channel+i,datetext,entry[1],.85);
				i++;		 
			}
		}
	}

	update(data){
		this.component.components={};	//erase all components
		this.addRows(data);
		geid(this.name).innerHTML=this.component.getChildrenHtml();	//update visible result
	}

	addRow(id,cell1,cell2,fontsize){
		var row=new TableRow(id+"row");
		row.style="font-size:"+100*fontsize+"%;";
		this.component.addComponent(row);

		var cell=new TableCell(id+"cell1");
		cell.style="padding-top:0.2em;padding-bottom:0";
		row.addComponent(cell);	
		var namelabel=new TextLabel(id+"cell1text1",cell1);
		cell.addComponent(namelabel);	
		if(cell2) {
			var cellb=new TableCell(id+"cell2");
			cellb.style="padding-top:0.2em;padding-bottom:0";
			row.addComponent(cellb);	
			var namelabel2=new TextLabel(id+"cell2text1",cell2);
			cellb.addComponent(namelabel2);				
		}
	}

	getComponent(){return this.component;}
}






export {UserView};




