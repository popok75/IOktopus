
import {EventEmitter} from "../EventEmitter.js";

import {Component, ComponentFactory, TextInputComponent, SelectComponent, DivComponent, Panel,Table,TableRow ,TableCell, TextLabel, LabelBinaryComponent, CheckBoxBinaryComponent, SelectBinaryComponent, ClickableTextLabel} from "./Component.js";

import {LogGraphGroup } from "./LogDygraph.js";

var geid=document.getElementById.bind(document);
var lightgray="#BBBBBB";




///////////////
class GenericNodeView extends EventEmitter {
	constructor(name,data){
		super();
		this.name=name;
		this.component=new Component(name);
//		add text label for name
//		add interactive text field/radio button for value with optional title 'value
		var row=new TableRow(name+"row");
		this.component.addComponent(row);		
		var cell=new TableCell(name+"cellname");
//		cell.style="padding:5px";
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
		 */
	};

	getComponent(){
		return this.component;
	}
}



class VarView extends Component {
	constructor(nodename,varname,data,emitter){
		super(nodename+varname+"varview");
		this.lines=1;

		this.nodename=nodename;
		this.varname=varname;

		var valuelabel=new TextLabel(nodename+varname+"label",varname+":",{nodiv:true});
		valuelabel.style="font-size:70%;color:darkgray;";
		this.addComponent(valuelabel);
		this.valviews={};
		if(typeof(data[varname])=="object") {
			new PlusMinusElements(this,nodename,varname,data);
			for(var subvar in data[varname]) {
				var value=data[varname][subvar];
				var path=varname+'/'+subvar;
				var varview=new ValueView(nodename,path,value, data,emitter);
				this.addComponent(varview);this.lines++;
				this.valviews[subvar]=varview;
			}	
		} else {
			var varview=new ValueView(nodename,varname,data[varname],data,emitter);
			this.addComponent(varview);this.lines++;
			this.valviews[""]=varview;
		}

	}

	update(data){
		for(var k in this.valviews) {
			if(k=="") this.valviews[k].update(data[this.varname],data);
			else this.valviews[k].update(data[this.varname][k],data);
		}
		/*	if(typeof(data[this.varname])=="object") {
			 console.log();
		} else {
			 console.log();
		}*/
	}
}


class ValueView extends DivComponent {
	constructor(nodename,varname,value, data,emitter){	
		super(nodename+'/'+varname+"#value");
		var	formatter=new Formatter();
		this.formatter=formatter;
		this.varname=varname;
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
		this.valuecomp=valuecomp;
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
	
	
	update(value0,data){
		var value=this.formatter.format(value0,this.varname,data);
		//var format=this.formatter.formatDetect(value0,this.varname);
		if(this.formatter.syntaxDetect(value0,this.varname)=="enum") this.valuecomp.update(this.formatter.getEnumCases(value));
		else this.valuecomp.update(value);
	}
	
	
}







class Formatter {
	constructor(){
		/*this.symbols={"greaterorequal":">=","less":"<"};
		this.symbolsnamepattern=["operator"];
		this.boolnamepattern=["active","cyc","conditionmet"];
		this.timestampnamepattern=["ts"];*/
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
		var concatab= globalThis.globaldefs.data.concattab;
		 
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



export {GenericNodeView};




