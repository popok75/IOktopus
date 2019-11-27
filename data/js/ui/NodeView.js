
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
		this.row=row;
		var namelabel=new TextLabel(name+"name",name.split("/").pop(),{nodiv:true});
		this.arrows=new ArrowButtons(name,data);
		cell.addComponent(this.arrows);
		cell.addComponent(namelabel);
		this.refreshVarViews(data)
	}

	
	
	refreshVarViews(data, donterase){
		var arrows=this.arrows;
		for(var k in this.varviews) {
			if(k in data) this.varviews[k].update(data);
			else if(!donterase){// remove
				this.row.removeComponent(this.varviews[k].parent);
				//this.varviews[k].selfRemove();
				// to be removed it need to be removed from parent component
				arrows.removeVarView(this.varviews[k].name);
				delete this.varviews[k];
					
			}
		}// must also remove the ones that disappeared
		
		
		var selected=arrows.getOrder(data);
		var ordered=selected[0];
		ordered=ordered.concat(selected[1]);
		ordered=ordered.concat(selected[2]);
		if(!this.varviews) this.varviews={};
		
		
		for(var i in ordered){
			var t=ordered[i];
			// check if we dont have it already
			if(this.varviews[t]) continue;	// if exist just update
			
			var last;
			for(var j=0;j<i;j++){// we want to insert the next cell at the right place : after past items of ordered, before any other item
				var found=false;
				for(var k in this.row.components){ 
					if(this.row.components[k].name==(this.name+"-cell-"+ordered[j]))
						{found=true;last=this.row.components[k];break;}
				}
			}
			if(!(t in data)) continue;
			
			var cell2=new TableCell(this.name+"-cell-"+t);	// if does not exist create a new cell
			
			if(found) this.row.addComponentAfter(cell2,last);
			else this.row.addComponent(cell2);			// and add it at the right place
			
			var varview=new VarView(this.name, t, data, this);	
			varview.parent=cell2;
			arrows.addVarView(t, cell2);			// put a varview inside
			cell2.updateStylesFromString("text-align:center;");
			if(varview.lines>2) cell2.updateStylesFromString("padding:0px 3px 0px 3px;");else cell2.updateStylesFromString("padding:3px 5px 3px 5px;");
			if(!selected[0].find(function(e){return t==e;})) {
				cell2.updateStylesFromString("display:none;");
			}
			
			cell2.addComponent(varview);			
			this.varviews[t]=varview;
		}
	}

	update(data, partialupdate){
		 this.refreshVarViews(data,partialupdate);
		return;
		/*for(var k in this.varviews) {
			this.varviews[k].update(data);
		}*/




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
				var valview=new ValueView(nodename,path,value, data,emitter);
				this.addComponent(valview);this.lines++;
				this.valviews[subvar]=valview;
			}	
		} else {
			var valview=new ValueView(nodename,varname,data[varname],data,emitter);
			this.addComponent(valview);this.lines++;
			this.valviews[""]=valview;
			
		}

	}
	

	update(data){
		if(!(this.varname in data)) return;	// no data for this particular variable
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
	constructor(nodename,varname,value0, data,emitter){	
		super(nodename+'/'+varname+"#value");
		var	formatter=new Formatter();
		this.formatter=formatter;
		this.varname=varname;
		var value=formatter.format(value0,varname,data);

		var st=formatter.getStyle(value0,varname);
		var valuecomp, mevalueview=this;

		// get a list of favorite/compatible formats for this var

		var favorites=formatter.getCompatibleComponents(value,varname);
		var currentfav=0;
		// create most favorite
		valuecomp=formatter.createComponent(favorites[currentfav],nodename+'/'+varname,value);
		this.valuecomp=valuecomp;
		this.initval=value;
		valuecomp.postnotify=function(){
			// become gray if modified until enter or focusout, then it is saved to the server, become orange, until confirmed
			if(valuecomp.confirmed) return ;
			valuecomp.stopremoteupdate=true;
			valuecomp.updateStyle("color","lightgray");
//			console.log(valuecomp.name);		
		};
		valuecomp.cancel=function(){
			valuecomp.update(this.initval);		 
		}.bind(this);
		var that=this;
		valuecomp.confirm=function(tval){
			//following test should be in component
			if(valuecomp.textvalue && valuecomp.textvalue==valuecomp.initval) {valuecomp.postupdate();return;} // no change in text
			// unless it did not change, it should become blue and be sent to the server
			valuecomp.updateStyle("color","blue");
			var obj={};
			obj[valuecomp.name]=formatter.encode(tval,varname, data);				
			// model modification require reverse formatting the data from what the user see to what the server understand
			valuecomp.confirmed=true;
			valuecomp.stopremoteupdate=false;
			valuecomp.updateProperty("disabled","disabled");//  disable the component
		
			emitter.emit("userChange",obj);	
			console.log(that);
			// will wait the node controller to provide an update, maybe have a special userconfirm/usercancel & serverconfirm/serverconfirm function ?
		}
		
		valuecomp.postupdate=function(){
			valuecomp.updateStyle("color","black");
			valuecomp.stopremoteupdate=false;
			valuecomp.updateProperty("disabled");	//remove property
			valuecomp.confirmed=false;
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
		this.initval=value;
		if(this.valuecomp.stopremoteupdate) return;	//dont update if user is editing
		if(this.formatter.syntaxDetect(value0,this.varname)=="enum") this.valuecomp.update(this.formatter.getEnumCases(value));
		else this.valuecomp.update(value);
		
		var st=this.formatter.getStyle(value0,this.varname);
		if(st) this.valuecomp.updateStyles(st);
	}
	
	
}





function fetchPath(path, obj) {
	  var current=obj; 
	  path.split('/').forEach(function(p){ current = current[p]; }); 
	  return current;
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
	isTransformedSymbol(val){		
		for(var s in this.symbols) 
			if(val.indexOf(this.symbols[s])>-1) return true;
		return false;
	}
	revertSymbols(val){		
		for(var s in this.symbols) 
			if(val.indexOf(this.symbols[s])>-1) val=val.replaceAll(this.symbols[s],s);
		return val;
	}

	formatDetect(value,varname){
		//should detect type from var name/varname pattern too
		
		if(!value) return "";
		if((value[0]=='/' || value.substring(0,2)=="./")) return "link";

		if(Number(value) > 946684800) return "ts";//21st century
		for(var s in this.timestampnamepattern) if(varname.indexOf(this.timestampnamepattern[s])>-1) return "ts";

		for(var s in this.symbols) if(value.indexOf(s)>-1) return "sym";
		for(var s in this.symbolsnamepattern) if(varname.indexOf(this.symbolsnamepattern[s])>-1) return "sym";

		for(var s in this.boolnamepattern) if(varname.indexOf(this.boolnamepattern[s])>-1) return "bool";
		return "";
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

	encodeEnumCases(value,values) {
		var cases=this.getEnumCases(values);
		var fval=value+"{", virg=0;
		for(var k in cases) {
			if(fval.indexOf(cases[k])>-1) continue;
			if(virg) fval+=",";
			fval+=cases[k];
			virg=1;
		}
		fval+="}";
		return fval;
	};
	
	encode(value,varname,data) {
		var dval=data[varname];
		var synt=this.syntaxDetect(dval,varname);
		if(this.isTransformedSymbol(value)) value=this.revertSymbols(value);
		if(synt=="enum") return this.encodeEnumCases(value,dval);
		return value;
	}
	
	format(value,varname,data){
		var format=this.formatDetect(value,varname);
		if(format=="ts" && value>0) return new Date(Number(value)).toLocaleString();	// time is in seconds 
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

		//console.log(ddata);

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
	
	updateData(data){
		this.order=this.getOrder(data);
		
		var k=this.order.length-1;
		for (;k>0;k--) if(this.order[k].length) break;
		//if(k==0) k=this.order.length-1;
		this.maxorder=k;		
	}
	updateArrows(order){
		if(this.currentlevel==0) this.arrowless.updateStyle("color","lightgray");	//disable cursor too
		else this.arrowless.updateStyle("color","");
		if(this.currentlevel>=(this.maxorder)) this.arrowmore.updateStyle("color","lightgray");
		else this.arrowmore.updateStyle("color","");
	}
	updateVarviews(){
		for(var k in this.varviews){
			var n=this.varviews[k] ;
			function identical(e){return e==k;}
			for(var j=0;j<this.order.length;j++){
				if(this.currentlevel>=j && this.order[j].find(identical)) n.updateStyle("display","");
				if(this.currentlevel<j && this.order[j].find(identical)) n.updateStyle("display","none");
			}				
		}
		 
	}
	
	
	constructor(name,data){
		super();
		this.updateData(data);
		
		this.arrowless=new TextLabel(name+"arrowless","&#9665;",{nodiv:true});
		this.arrowmore=new TextLabel(name+"arrowmore","&#9655;\t",{nodiv:true});

		var mearrow=this;
		mearrow.currentlevel=0;
		
		this.arrowless.changed=(function(){
			console.log("less");
			if(this.currentlevel>0) {
				this.currentlevel--;
				while(this.order[this.currentlevel].length==0 && this.currentlevel>0) this.currentlevel--;
				this.updateVarviews();
				this.updateArrows(this.order);
			} else return;
		}).bind(this);
		
		this.arrowmore.changed=(function(){
			console.log("more"+mearrow);
			if(this.currentlevel<this.maxorder) {
				this.currentlevel++;
				while(this.order[this.currentlevel].length==0 && this.currentlevel<this.maxorder) this.currentlevel++;
			} else return;
			this.updateVarviews();
			this.updateArrows(this.order);
		}).bind(this);

		this.arrowmore.selfsubscribe();
		this.arrowmore.style="cursor: pointer;";
		this.arrowmore.extra=" onclick='window.objectChange(this)'";
		//if(this.currentlevel==2) this.arrowmore.style+="color:lightgray;";	//disable cursor too

		this.arrowless.selfsubscribe();
		this.arrowless.style="cursor: pointer;"; 
		this.arrowless.extra=" onclick='window.objectChange(this)'";
		//if(this.currentlevel==0) this.arrowless.style+="color:lightgray;";	//disable cursor too

		this.updateArrows(this.order);
		
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
	removeVarView(name){
		if(!name) return;
		if(name in this.varviews) delete this.varviews[name];
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




