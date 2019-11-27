


//Notes : - this file should contain only generic logdygraph
//- code could be improved
import {Component,Panel, DivComponent,Button,TextLabel } from "./Component.js";

import {} from "./dygraph.min.js";	// no-copy alternative export method : the original object is exported as global only a copy is retreived with import
//import * as syncro from "./synchronizer.js";

import {downV3,moveV3,upV3,clickV3,dblClickV3,scrollV3} from "./interaction.js";
import {} from "./synchronizer.js";	// synchronizer will just modify global variable Dygraph, so no need to import anything

import {mixColors, generateColors} from "./ColorUtils.js";


//Dygraph=window.Dygraph;
var geid=document.getElementById.bind(document);

var ONEDOTMSAROUND=30000;
var USECSV=false;
var USERANGESUBSET=true;

function changeStylesheetRule(stylesheet, selector, property, value) {
	selector = selector.toLowerCase();
	property = property.toLowerCase();
	value = value.toLowerCase();
	for(var i = 0; i < stylesheet.cssRules.length; i++) {
		var rule = stylesheet.cssRules[i];
		if(rule.selectorText === selector) {rule.style[property] = value;return;}
	}
	stylesheet.insertRule(selector + " { " + property + ": " + value + "; }", 0);
}
 
class LogGraphGroup {	//interface the data and the graphs
	constructor(name,component,nodes){
		this.name=name;
		this.component=component;
	}


	updateConfig(nodes,labels){
		this.colorPerLabel={};
		this.unitPerLabel={};
		// group by unit
		var groups={units:[],perunit:{}};
		for(var i in labels) {
			if(labels[i]=="Date") continue;
			var found=0,u="";
			for(var k in nodes) if(k==labels[i]) found=nodes[k];
			if(found) u=found["unit"];
			
			if(!groups.perunit[u]) {groups.perunit[u]=[];groups.units.push(u);}
			groups.perunit[u].push(labels[i]);
		}

		var config=[];
		var c=0,op=0;

		for(var j in groups.units) {	// each two group will make a graph
			//var cindex=Math.floor(op/2);
			var y=[],ax={};
			var i=groups.units[j];
			for(var gm in groups.perunit[i]) {
				y.push({name:groups.perunit[i][gm],unit:i,color:this.colorset[c]});
				this.unitPerLabel[groups.perunit[i][gm]]=i;
				this.colorPerLabel[groups.perunit[i][gm]]=this.colorset[c];
				c++;}
			ax={unit:i};
			if(!(op%2)){
				var opt={series:{y:y,y2:[]}, axis: {y:ax,y2:{unit:""}}};
				config.push(opt);
			} else {
				config[config.length-1].series.y2=y;
				config[config.length-1].axis.y2=ax;
			}
			op++;
		}

		this.groups=groups;

		this.config=config;
	}


	init(rawdata,labels,nodes){
		// adapt colors to data
		var size=labels.length;
		this.colorset=generateColors(size);	// better to generate new color on demand, i.e. according to data labels length 

		// classify data into groups
		this.updateConfig(nodes,labels);

		//	this.config.labels=labels;

//		this.graphid=0;
		this.graphs=[];
		
		// there is problem in Component.js with adding more graph divs after initializing so we separate in two steps	
		for(var k in this.config){
			var graph=new LogDygraph(this.name+this.graphs.length,rawdata.length);// this object will create the canvas tag on creation
			this.component.addComponent(graph.getComponent());
			this.graphs.push(graph);
		}

		var comp=new DivComponent();	// add + / - buttons
		var bp= new Button(this.name+"plus","+");
		var bm= new Button(this.name+"minus","-");
		var l= new TextLabel(this.name+"sep","/",{nodiv:true});
		comp.addComponent(bp);
		comp.addComponent(l);
		comp.addComponent(bm);
		this.plusminus=comp;
		bp.notify=function(){
			// create a new graph
			var graph=new LogDygraph(this.name+this.graphs.length,rawdata.length); 
			this.component.addComponent(graph.getComponent()); 
			// this has a problem as the add component refreshtml is destructive
			// add it 
		};
		
		this.component.addComponent(comp);
		
		for(var k in this.config){
			this.graphs[k].init(rawdata, labels, this.config[k],this.colorPerLabel, this.unitPerLabel);
		}

		 
		this.graph=graph;
	};


	prepareData(raw, labels, config){		// in theory, it is useless and a waste of time as dygraph can ignore irrelevant columns
		var extract=[],tokeep={},nlabels=[];
		for(var i in labels) {
			var lab=labels[i];
			if(lab=="Date") {tokeep[lab]=1;nlabels.push(lab);continue;}	//we always keep "Date" column
			function ident(pos){return lab==pos.name;};
			var t1=config.series.y.find(ident);
			var t2=config.series.y2.find(ident);
			if(t1 || t2 ) {nlabels.push(lab);tokeep[lab]=1;}
		}
		var col=0;
		for(var j in labels){
			var l=labels[j];
			if(tokeep[l]) {
				for(var k in raw) {
					if(!extract[k]) extract[k]=[];
					extract[k][col]=raw[k][j];
				}
				col++;
			}
		}
		return {sublabels:nlabels, subdata:extract}
		//return extract;
	}


	update(rawdata,labels,nodes){
		//var data=this. prepareData(rawdata);
		// must update the config labels color 
		this.updateConfig(nodes,labels);			// TODO : should update, not recreate
		// must update the ranges if needed add/remove ranges
		// must filter out data per graph
		for(var k in this.config) {
			if(this.graphs[k]) this.graphs[k].update(rawdata,labels,this.config[k],this.colorPerLabel, this.unitPerLabel);
			else {
				var graph=new LogDygraph(this.name+this.graphs.length,rawdata.length);// this object will create the canvas tag on creation
				this.component.addComponentBefore(graph.getComponent(),this.plusminus);
				this.graphs.push(graph);
				this.graphs[k].init(rawdata, labels, this.config[k],this.colorPerLabel, this.unitPerLabel);
			}
		}
	};
	 

};
 


export {LogGraphGroup};
 


function legendFormatter(data) { 
	return ""; 
};

function labelformatter(unit,axisunit){ 
	this.unit=unit;
	this.axisunit=axisunit;
	this.valueFormatter= function(y) {
		var str=y.toFixed(2);
		if(this.unit) str+= this.unit;
		return str; 
	}.bind(this);
	
	this.axisLabelFormatter= function(y) {
		var str=y.toFixed(2);
		if(this.axisunit) str+= this.axisunit;
		return str; 
	}.bind(this);
	
	this.setUnits= function(unit,axisunit){ 
		this.unit=unit;
		this.axisunit=axisunit;
		}.bind(this);
		
	this.pixelsPerLabel =20 ;
};

class MainDygraph {

	getSeries(data,labels,rawlabels){
		this.subdata=getMultiSub(data,labels,rawlabels);
		if(this.usecvs) return toCSV( this.subdata);
		return this.subdata;
	}

	updateConfig(config,opt){
		var vistab=this.getVisibility(this.labels,config);		
		var seriesobj=this.getSeriesobj(config);
		this.formatters[0].setUnits(config.axis.y.unit,config.axis.y.unit);
		this.formatters[1].setUnits(config.axis.y2.unit,config.axis.y2.unit);
		//var formatters=this.setupAxis(config);
		if(!opt) opt={};
		Object.assign(opt,{visibility: vistab,series:seriesobj});//, highlightSeriesOpts: {axes: { y: formatters[0],y2: formatters[1]}}}; 
			
		this.maingraph.updateOptions( opt,false);
		this.maingraph.resize();
	};

	update(rawdata,labels,config){
		this.labels=labels;
		this.rawdata=rawdata;
		var opt={'file': rawdata,labels:labels};	//update visibility ?
		this.updateConfig(config,opt);
	}
	
	constructor(name, data, labels, config, colorset){

		function generalConfig(){//need only to be executed once for all graphs
			changeStylesheetRule(document.styleSheets[0], ".dygraph-legend","text-align","left");
			changeStylesheetRule(document.styleSheets[0], ".dygraph-legend","margin-left","70px");
			//  	changeStylesheetRule(document.styleSheets[0], ".dygraph-legend","display","content");
			changeStylesheetRule(document.styleSheets[0], ".dygraph-legend","background","white");
			//	changeStylesheetRule(document.styleSheets[0], ".dygraph-legend","position","absolute");
			//	changeStylesheetRule(document.styleSheets[0], ".dygraph-legend","position","absolute");
			changeStylesheetRule(document.styleSheets[0], ".dygraph-legend","z-index","10");
//			changeStylesheetRule(document.styleSheets[0], ".dygraph-legend","width","200px");
			Dygraph.ANIMATION_DURATION=0;
		}
		function setupAxis(config){
			// set color & style of axis label	-> does these apply to all graphs in same page ?
			var yunit="",y2unit="";
			if(config.axis){
				if(config.axis.y2 && config.axis.y2.color) changeStylesheetRule(document.styleSheets[0], ".dygraph-axis-label-y2","color",config.axis.y2.color);
				if(config.axis.y && config.axis.y.color) changeStylesheetRule(document.styleSheets[0], ".dygraph-axis-label-y","color",config.axis.y.color);
				if(config.axis.y2 && config.axis.y2.unit) y2unit=config.axis.y2.unit;
				if(config.axis.y && config.axis.y.unit) yunit=config.axis.y.unit;
			}

			var vunit="",v2unit="";
			if(config.series.y[0].unit) vunit=config.series.y[0].unit;
			if(config.series.y2[0] && config.series.y2[0].unit) v2unit=config.series.y2[0].unit;
			var vf1=new labelformatter(vunit,yunit);	//for multiple on one axis, should find a unit per line
			var vf2=new labelformatter(v2unit,y2unit);
			return [vf1,vf2];
		}
		function getSeriesobj(config){
			//var labels0=labels.slice();	//keep original separate	// copy y,y2 and sublabels
			var series0={};
			for(var n in config.series.y) {series0[config.series.y[n].name]={axis: 'y'};}
			for(var n in config.series.y2) {series0[config.series.y2[n].name]={axis: 'y2'};}
			//for(var n=1;n<labels0.length;n++) if(!(labels0[n] in series0)) labels0.splice(n,1);
			return series0;
		}
		function getColors(labels,colorset){
			var vcolorsall=[];	// copy colors from config object into an array matching labels
			for(var n=1;n<labels.length;n++) vcolorsall.push(colorset[labels[n]]);	
			return vcolorsall;
		}
		function getVisibility(labels, config){
			var ret=[];
			for(var n=1;n<labels.length;n++){
				var nname=labels[n], hide=false;
				function check(serie){
					for(var k in serie) 
						if(serie[k].name==nname/* && serie[k].hide*/) {if(!serie[k]["hide"]) return false; else return serie[k].hide;}
					return undefined;
				}		
				var nhide=check(config.series.y);
				if(typeof(nhide)!="undefined") hide=nhide; 
				else {
					nhide=check(config.series.y2);
					if(typeof(nhide)!="undefined") hide=nhide; 
					else hide=true;
				}
				ret.push(!hide);
			}
			return ret;
		}

		this.getVisibility=getVisibility;
		this.getSeriesobj=getSeriesobj;
		this.setupAxis=setupAxis;
		this.labels=labels;

		this.name=name;
		generalConfig();
		var formatters=setupAxis(config); 	//use units
		this.formatters=formatters;
		var seriesobj=getSeriesobj(config);	//name per axis
		var vcolorsall=getColors(labels,colorset); // color per name
		var dataseries=data;//this.getSeries(data,labels0,labels);
		var vistab=getVisibility(labels,config);

		var title0= config.title | " ";
		this.im={'mousedown' : downV3,'mousemove' : moveV3,'mouseup' : upV3,'click' : clickV3,'dblclick' : dblClickV3,
				'mousewheel' : function(event, g, context){	scrollV3(event, g, context);}};
	//	var height=30; 	
		var opt={
				labels: labels,
				animatedZooms: false,
				visibility: vistab,
				series: seriesobj,
				//      customBars: true,
				title: title0,
				pointSize: 1.5,
				connectSeparatedPoints: true,
				drawPoints: true,
				colors: vcolorsall,
				
				showRangeSelector: true,
				rangeSelectorHeight: 20,
				//showLabelsOnHighlight :false,
				xAxisHeight: 14,
				
		//		ylabel: config.axis.y.unit | "", y2label: config.axis.y2.unit | "",
				labelsSeparateLines: true,
				labelsShowZeroValues: false,
				highlightSeriesBackgroundAlpha : 0.8,
				highlightSeriesOpts: { 
					strokeWidth: 2,
					strokeBorderWidth: 1,
					highlightCircleSize: 4 },
					legendFormatter: legendFormatter,
					axes: { y: formatters[0], 
						y2: formatters[1], 
						axisLabelWidth: 100
					},
					legend: 'always',//showRangeSelector: true,
					interactionModel: this.im,
					showRoller: true//, errorBars: true
		};
		if(dataseries.length==1) {
			var min =new Date(dataseries[0][0]).getTime()-ONEDOTMSAROUND, max=new Date(dataseries[0][0]).getTime()+ONEDOTMSAROUND;
			if(min<0) min=0;
			opt.dateWindow=[new Date (min),new Date(max)];
		}
		/*
		opt.underlayCallback= (context) => {context.save();};
		opt.drawCallback= (dygraph) => {dygraph.canvas_ctx_.restore();};
		 */
		var elem=geid(this.name);

		//	this.maingraph=new Dygraph(elem,dataseries,opt); 
		this.maingraph=new Dygraph(elem,data,opt);

		window.onresize = function(event) {	// do not resuze unless we do this
			this.maingraph.maindiv_.children[0].style.width="100%";
			this.maingraph.resize();

			for(var r in this.ranges) {
				this.ranges[r].maindiv_.children[0].style.width="100%";
				this.ranges[r].resize();
			}


		}.bind(this); 
	}


}



class PanelLegend {
	constructor(cname0, labels0,config,colorset){

		function addLine(cname,n,names,colorst,states){
			 
			var it=names.find(function(t){return t.name==n;});
			
			if(it) states[n]=1; else states[n]=0;
			
			var row="<tr id='"+cname+"line-"+n+"'style='color:";
			var halfcol="#888888",fullcol="#000000";
			if(colorset[n]) {halfcol=mixColors([colorst[n],"#888888","#888888"]);fullcol=colorst[n];}
			if(it) row+=fullcol+";font-weight:bold;";
			else row+=halfcol+";";
			row+="'><td id='"+cname+"name-"+n+"'>"+n+"</td>";
			row+="<td id='"+cname+"value-"+n+"'></td>";
			row+="<td><input type=checkbox style='transform: scale(.8);margin:0;vertical-align:middle;' id='"+cname+"cb-"+n+"' value="+n;
			if(it) row+=" checked";
			row+=" onclick='objectChange(this,event)'></td></tr>";		 
			subscribeObject(cname+"cb-"+n,this);
			return row;
		}
		
		function addtable(cname,floatitem,names,colorst,states){

			var res="<div style='float:"+floatitem+";'><table style='font-size:80%;border-spacing:0'><tbody id="+cname+">";
			for(var k=1;k<labels.length;k++){
				var n=labels[k];
				res+=addLine.bind(this)(cname,n,names,colorst,states);
			}
			res+="</tbody></table></div>";
			return res;
		}
		var labels=labels0.slice(0).sort(function(a,b){//var a=ao.label,b=bo.label;
			if(a<b) return -1; else if(a==b) return 0; else	if(a>b) return 1;
		});
		this.states={y:{},y2:{}};
		this.addLine=addLine.bind(this);
		
		var item=geid(cname0);
		this.labels=labels;

		var html=addtable.bind(this)(cname0+"y","left",config.series.y,colorset,this.states.y);
		html+=addtable.bind(this)(cname0+"y2","right",config.series.y2,colorset,this.states.y2);

		this.colorset=colorset;
		this.cname0=cname0;

		item.innerHTML=html;
		 
	};
	
	getStates(){return this.states;}
	
	update(nlabels,config,colorset,unitset){
		
		console.log(item);
		
		for(var k in this.labels){
			var found=0;
			var n=this.labels[k];
			for(var j in nlabels) if(nlabels[j]==this.labels[k]) {found=1;break;}
			if(!found) {
				var cname=this.cname0+"y";
				var child=geid(cname+"line-"+n);
				var item=geid(cname);
				item.removeChild(child);
				cname=this.cname0+"y2";
				child=geid(cname+"line-"+n);
				item=geid(cname);
				item.removeChild(child);
			}
		}
		for(var j in nlabels) {
			var found=0, n=nlabels[j];
			for(var k in this.labels) if(nlabels[j]==this.labels[k]) {found=1;break;}
			if(!found) {
				//create line
				var cname=this.cname0+"y";
				var html=this.addLine(cname,n,config.series.y,colorset,this.states.y)
				var nelement=document.createElement("TR");
				nelement.innerHTML=html;
				var item=geid(cname);
				item.appendChild(nelement);
				
				cname=this.cname0+"y2";
				html=this.addLine(cname,n,config.series.y2,colorset,this.states.y2)
				nelement=document.createElement("TR");
				nelement.innerHTML=html;
				item=geid(cname);
				item.appendChild(nelement);
				
			}
		}
		
		this.labels=nlabels;
	};

	setSelected(checked,name,axis){
		if(checked) this.states[axis][name]=1; else this.states[axis][name]=0; 
		var it2=geid(this.cname0+axis+"line-"+name);
		if(!it2) return;
		if(checked){				 
			it2.style.color=this.colorset[name];
			it2.style["font-weight"]="bold";			 
		} else {			 
			it2.style.color=mixColors([this.colorset[name],"#888888","#888888"]);
			it2.style["font-weight"]="";
		}
	}
	changed(id){
		var notifs=[];
		var axis="y", item =geid(id);
		var name=item.value;
		if(item.id==this.cname0+axis+"cb-"+name) axis="y"; else axis="y2";
		if(item.checked) {
			this.setSelected(true,name,axis);
			var otheraxis="y2";
			if(axis=="y2") otheraxis="y";
			var otherycb=geid(this.cname0+otheraxis+"cb-"+name);
			if(otherycb){
				otherycb.checked=false;
				this.setSelected(false,name,otheraxis);
				//if(this.listener && this.listener.notify) this.listener.notify(otheraxis,name,false);
				notifs.push([otheraxis,name,false]);
			}
		}
		else this.setSelected(false,name,axis);

		console.log();
		notifs.push([axis,name,item.checked]);
		if(this.listener && this.listener.notify) this.listener.notify(notifs);
	}
};



class LogDygraph {
	constructor(name,datalength){
		this.name=name;
		this.ranges=[];
		this.usecsv=USECSV;
		this.userangesubset=USERANGESUBSET;

		var n=this.name+"main";
		this.component= new Component(n);	
		this.createCanvas(n,datalength);
	}

	getComponent(){return this.component;}

	createCanvas(canvasname,datalength){
		var ncomponent= new Panel(canvasname+"container","",""+datalength+" time point");	
		
		var component= new Component(canvasname);
		component.tag="div";
		component.style="width:100%;"//height:200px;";	 
			//this.component.addComponent(component);
		// create a div for the legend
		// create a div for the main graph
		var componentl= new Component(canvasname+"legend");
		componentl.tag="div";
		componentl.style="width:100%;display:flow-root"//height:200px;";	 
			//this.component.addComponent(componentl);
		ncomponent.addComponent(componentl);	
		ncomponent.addComponent(component);		
		this.component.addComponent(ncomponent);
		
	}

	
	applyLegend(config){
		var states=this.legend.getStates();
		for(var n in states.y){
			if(!states.y[n]) {	// if not checked, add hide property
				for(var l in config.series.y) 
					if(config.series.y[l].name==n) config.series.y[l].hide=1;
			}
		}
		for(var n in states.y2){
			if(!states.y2[n]) {	// if not checked, add hide property
				for(var l in config.series.y2) 
					if(config.series.y2[l].name==n) config.series.y2[l].hide=1;
			}
		}
	}
	
	update(rawdata,labels,config, colorset,unitset){
		geid(this.name+"maincontainer").children[0].children[0].innerHTML=""+rawdata.length+" time point";// should move this to panel.setlegend
		
		//TODO : should update legend in case of
		this.legend.update(labels,config,colorset,unitset);
	
		this.applyLegend(config);
	 
		this.maingraph.update(rawdata,labels,config);
	}
	

	init(data, datalabels, config,colorset, unitset)
	{	
		// save parameters
		this.config=config;
		this.data=data;		
		var obj=this;
		var n=this.name+"main";

		this.legend=new PanelLegend(n+"legend",datalabels,config,colorset);
		this.legend.listener={notify:function(notifs){//axis,name, state){
			for(var k in notifs){
				var axis=notifs[k][0];
				var name=notifs[k][1];
				var state=notifs[k][2];
				console.log(obj);
				console.log(datalabels);
				function func(t){return t.name==name;};
				var tab,othertab;
				if(axis=="y") {tab=config.series.y;othertab=config.series.y2;}
				else if(axis=="y2") {tab=config.series.y2;othertab=config.series.y;}
				var index=-1;
				for(var k in tab)if(tab[k].name==name) {index=k;break;}
				if(index!=-1) {
					var it=tab[index];
					it.hide=!state;//if(!state) tab.splice(index,1);
				}
				else {
					tab.push({name:name, color:colorset[name], hide:!state, unit:unitset[name]});
					var k=0;
					for(;k<othertab.length;k++) if(othertab[k].name==name) break;
					if(k<othertab.length) othertab.splice(k,1);
				}	//if(this.maingraph) this.maingraph.setVisibility()				
			}
			function updateUnits(){
				// unit has to be among the selected graphs
				// if all have same unit different from
				var unity=config.axis.y.unit; 
				var funitsy={}, funitsy2={};
				for(var k in config.series.y) if(!config.series.y[k].hide) funitsy[config.series.y[k].unit]=1;
				for(var k in config.series.y2) if(!config.series.y2[k].hide) funitsy2[config.series.y2[k].unit]=1;
				if(!funitsy[config.axis.y.unit] && Object.keys(funitsy).length) config.axis.y.unit=Object.keys(funitsy)[0];
				if(!funitsy2[config.axis.y2.unit] && Object.keys(funitsy2).length) config.axis.y2.unit=Object.keys(funitsy2)[0];
			}
			updateUnits();
			obj.maingraph.updateConfig(config);
		}};

		this.maingraph=new MainDygraph(n, data, datalabels, config,colorset);
	};
};















	/** AFTER THIS POINT ALL THE CODE IS NOT USED*/
/*
 * 
 function legendFormatter(data) {/*
	function updateLegend(axis,ser){
		var l=this.maindiv_.id+"legend"+axis+"name-"+ser.label;
		var itm=geid(l);
		if(itm){
			var cb=geid(this.maindiv_.id+"legend"+axis+"cb-"+ser.label);
			if(cb && cb.checked) {
				itm.style.color=ser.color;
				itm.style["font-weight"]="bold";
			}
		}
	}

	for(var k in data.series){		
		updateLegend.bind(this)("y",data.series[k]);
		updateLegend.bind(this)("y2",data.series[k]);
	};*/
	/*	return ""//html; //	return data;
}

/**//*
	var sorted=data.series.sort(function(ao,bo){
		var a=ao.label,b=bo.label;
		if(a<b) return -1; else if(a==b) return 0; else	if(a>b) return 1;
	});
	if (data.x == null) {// This happens when there's no selection and {legend: 'always'} is set.
		var legend= '' + sorted.map(function(series) {return '<span style="font-weight: bold; color: '+series.color+';">'+series.dashHTML +  series.labelHTML+'</span>' }).join('<br>');
		return legend;
	}
	var html = this.getLabels()[0] + ': ' + data.xHTML;
	sorted.forEach(function(series) {
		var val=series.yHTML;
		if(val==undefined) val="-";	
		if (!series.isVisible) return;
		var labeledData = '<span style="text-align: left; color: '+series.color+';">'+series.dashHTML +'&nbsp;'+  series.labelHTML + ': ' + val+'</span>';
		if (series.isHighlighted) labeledData = '<b>' + labeledData + '</b>';
		html += '<br>' + series.dashHTML + ' ' + labeledData;
	});
	return "";//html;
}
	getRangeNamesY(name){
		var seriesnamesy=getPropertySubArray(this.config.series.y,"name");
		for(var ser in seriesnamesy) {if(seriesnamesy[ser]==name) {seriesnamesy=[name];break;}}
		if(seriesnamesy.length>1) seriesnamesy=[seriesnamesy[0]];
		return seriesnamesy;
	}
	getRangeNamesY2(name){
		var seriesnamesy2=getPropertySubArray(this.config.series.y2,"name");
		for(var ser in seriesnamesy2) {if(seriesnamesy2[ser]==name) {seriesnamesy2=[name];break;}}
		if(seriesnamesy2.length>1) seriesnamesy2=[seriesnamesy2[0]];
		return seriesnamesy2;
	}

	getRangeSeries(rangename, yaxiscount){
		if(!this.userangesubset) return this._getMainSeries();
		// possibly reduce data to one per axis (including the showed data) to improve performance ?
		var i=0;
		for(;i<this.labels.length;i++) if(this.labels[i]==rangename) break;
		if(i==this.labels.length) return 0;
		var series= getSubSeries(this.data, i,yaxiscount);
		if(series.length==1) {
			var d=new Date(series[0][0].getTime());
			series.push(series.slice(0)[0].slice(0));	// if there is only one dot, duplicate it to prevent draphic glitch
			series[0][0]=new Date(d.getTime()-ONEDOTMSAROUND);
			series[1][0]=new Date(d.getTime()+ONEDOTMSAROUND);
		}
		return series;
	}

	///////////////////
	getRangeLabels(name, seriesnamesy2){
		if(this.userangesubset){
			var labels0=['Date'];
			labels0.push(name);
			if(seriesnamesy2 && seriesnamesy2.length) labels0.push(seriesnamesy2[0]);
			return labels0;

		} else return this.labels;
	}
	getRangeSeriesObj(name,seriesnamesy, seriesnamesy2){ 
		var series0={};
		for(var n in seriesnamesy) {series0[seriesnamesy]={axis: 'y'};}
		for(var n in seriesnamesy2) {series0[seriesnamesy2]={axis: 'y2'};}	
		if(name in series0) series0[name].showInRangeSelector = true;
		return series0;
	}

	////////////////////////////////
	addRange(rangename, name,color){

		var seriesnamesy=this.getRangeNamesY(name);
		var seriesnamesy2=this.getRangeNamesY2(name);
		var ycount=0;
		if(seriesnamesy.length>0) ycount++;
		if(seriesnamesy2.length>0) ycount++;
		var dataseries=this.getRangeSeries(name,ycount);

		var config=this.config;
		var rangeid=this.ranges.length+1;
		var labels0=this.getRangeLabels(name, seriesnamesy2);
		var series0=this.getRangeSeriesObj(name,seriesnamesy, seriesnamesy2);

		var height=20, st=geid(rangename).style;
		if(st && st.height && st.height.substr(st.height.length-2,st.height.length)=="px") height=Number(st.height.substr(0,st.height.length-2));
		
		var opt={
				xAxisHeight: height,
				visibility: [false, false,false,false,false, false,false,false],
				plotter:function(e){},
				animatedZooms: false,
				connectSeparatedPoints: true,
				labels: labels0,
				drawPoints: false,
				fillGraph:false,
				series: series0,
				axes: {
					//		y:{valueRange: range[0]},	// does not work, no change of range selector
					//		y2:{valueRange: range[1]},
					drawAxis : false,
					x: {axisLabelFormatter:function(x){return "";}}
				},
				showRangeSelector: true,
				interactionModel: this.im,
				rangeSelectorHeight: height,
				showLabelsOnHighlight :false,
				rangeSelectorPlotStrokeColor: color,
				rangeSelectorPlotFillGradientColor: color,
				rangeSelectorPlotFillColor: "#FFFFFF"
		};

		opt.underlayCallback= (context) => {
			context.save();
		};
		opt.drawCallback= (dygraph) => {
			dygraph.canvas_ctx_.restore();
		};

		var rangegraph1=new Dygraph(
				geid(rangename),
				dataseries,
				opt
		);

		this.ranges.push(rangegraph1);
		var all=[];
		all.push(this.maingraph);
		for(var r in this.ranges) all.push(this.ranges[r]);
		if(all.length>1) Dygraph.synchronize(all, {selection: false,zoom: true,range: false});

	}


	updateMainGraph(){
		var dataseries=this._getMainSeries();
		var opt={ 'file': dataseries,labels: this.labels } ;
		// keep now visible
		var max=dataseries[dataseries.length-1][0].getTime();
		var min=dataseries[0][0].getTime();
		var recentermin=false,recentermax=false;
		if(this.maingraph.xAxisRange()[1]>=this.maingraph.xAxisExtremes()[1] &&  // most recent was visible untill now
				this.maingraph.xAxisRange()[1] < max)	// and the new most recent would be out of view
			recentermax=true;
		if(this.maingraph.xAxisRange()[0]<=this.maingraph.xAxisExtremes()[0] && // oldest data was visible untill now, 
				this.maingraph.xAxisRange()[0] > min)	// and it woud be out of view
			recentermin=true;
		if(recentermin || recentermax){
			if(!recentermin && this.maingraph.xAxisRange()[0]!=this.maingraph.rawData_[0][0]) min=this.maingraph.xAxisRange()[0];
			if(!recentermax && this.maingraph.xAxisRange()[1]!=this.maingraph.rawData_[this.maingraph.rawData_.length-1][0]) max=this.maingraph.xAxisRange()[1];
			opt.dateWindow=[min,max];
		}
		/*	{
			if(this.maingraph.xAxisRange()[0]!=this.maingraph.rawData_[0][0]) min=this.maingraph.xAxisRange()[0]; // follow the data unless the min of view has moved : not sure it is the best design choice
			opt.dateWindow=[min,max];	//adjust view max stays to max, min stay to min
		}
		if(this.maingraph.xAxisRange()[0]<=this.maingraph.xAxisExtremes()[0] && // oldest data was visible untill now, 
				this.maingraph.xAxisRange()[0] > min)	// and it woud be out of view
		{
			if(this.maingraph.xAxisRange()[0]!=this.maingraph.rawData_[0][0]) min=this.maingraph.xAxisRange()[0]; // follow the data unless the min of view has moved : not sure it is the best design choice
			opt.dateWindow=[min,max];	//adjust view max stays to max, min stay to min
		}
		 * /


		// update
		this.maingraph.updateOptions( opt,false);
		this.maingraph.resize();
	}

	updateRangeGraph(rangegraph,r){

		var name=rangegraph.getLabels()[1];

		var seriesnamesy=this.getRangeNamesY(name);
		var seriesnamesy2=this.getRangeNamesY2(name);
		var ycount=0;
		if(seriesnamesy.length>0) ycount++;
		if(seriesnamesy2.length>0) ycount++;
		var dataseries=this.getRangeSeries(name,ycount);

		//	var dataseries=this.getRangeSeries(name);
		if(!dataseries) return;
		var opt={ 'file': dataseries } ;
		// keep now visible
		if(!dataseries[dataseries.length-1]) {
			Console.log();
		}
		var max=dataseries[dataseries.length-1][0].getTime();
		/*
		if(this.maingraph.xAxisRange()[1]>=this.maingraph.xAxisExtremes()[1] &&  // most recent was visible untill now
				this.maingraph.xAxisRange()[1] < max)	// and it would disappear 	
			opt.dateWindow=[this.maingraph.xAxisRange()[0],max];	//adjust view
		 * /
		opt.dateWindow=[this.maingraph.xAxisRange()[0],this.maingraph.xAxisRange()[1]];
		// update
		rangegraph.updateOptions( opt);
		rangegraph.resize();
	}
/*
	update(data,labels){
		this.data=data;
		this.labels=labels;
		this.updateMainGraph();
		for(var r in this.ranges) this.updateRangeGraph(this.ranges[r],r);

//		this.maingraph.xAxisRange();
//		this.maingraph.xAxisExtremes();
		//dateWindow:

		/*		var extra=getSeries(data,seriesnamesy);	//should be seriesnamesy
		this.rangegraph1.updateOptions( { 'file': extra } );

		var extra2=getSeries(data,seriesnamesy2);	//should be seriesnamesy2
		this.rangegraph2.updateOptions( { 'file': extra2 } );
		 * /	}* /
};

function toCSV(data){
	var csv="";
	for(var i in data){
		var first=true;
		for(var j in data[i]){
			if(first) first=false;
			else csv+=", ";
			if(data[i][j]!=null) csv+=data[i][j];
			//else csv+="-";

		}
		csv+="\n";
	}
	return csv;
}

function getPropertySubArray(array,property){
	return array.map(function(item){ 
		return item[property];
	});
}



function getRatio(g){
	var tviewwidth=(g.xAxisRange()[1]-g.xAxisRange()[0]);
	var tdatawidth=g.rawData_[g.rawData_.length-1][0]-g.rawData_[0][0];
	var pointsvisible=g.rawData_.length*tviewwidth/tdatawidth;
	var ratio=100*pointsvisible/g.size().width;
	console.log(ratio);
	ratio=Math.round(ratio);
	return ratio;
};



function extractData(data,index){
	var res=[];
	for(var i in data){
		res.push([data[i][0],data[i][index]]);
	}
	return res;
}
/*
function getSerieByDate(serie, date){
	for (var index in serie){
		if(date<=serie[index][0] && date>=serie[index][0]) return serie[index];	// test date equality
	}
	return undefined;
}
 * /
function getMultiSub(data,sublabels,labels){
	var indexes=[];
	for(var i in sublabels) indexes.push(labels.findIndex(function(l){ return l==sublabels[i]}));
	return getMultiSubSeries(data,indexes);
}

function getMultiSubSeries(data,columnindexes){
	var series=[];
	for(var i in data) {
		var dok=false;
		for(var j in columnindexes) if(j>0 && data[i][columnindexes[j]]!=null) {dok=true;break;}
		if(dok) {
			var v=[];//[data[i][0]];
			for(var k=0;k<columnindexes.length;k++) v[k]=data[i][columnindexes[k]]
			series.push(v);
		}	 
	}
	return series;
}

function getSubSeries(data,column,yaxiscount){
	var series=[];
	for(var i in data) {
		if(data[i][column]!=null) {
			if(yaxiscount>1) series.push([data[i][0],data[i][column],0]);	// 0 is added for the other axis to keep a margin
			else series.push([data[i][0],data[i][column]]);
		}

	}
	return series;
}
/*
function getSeriesv1(data,names){
	var series=[];
	var i=1;
	var size=Object.keys(names).length+1;
	for(var n in names){
		var k=names[n];
		for(var l in data[k]){
			var datacell=data[k][l];
			var d=new Date(Number(datacell[0])*MILLIFACTOR);
			cell=[];
			for(var j=0;j<size;j++) cell.push(null);
			cell[0]=d;
			cell[i]=Number(datacell[1]);
			series.push(cell);
		}
		i++;
	}
	// sort 
//	series.sort(function(a,b) {return a[0]-b[0];});

	return series;
}
 *//*
function getCell(size,n){
	var cell2=[];
	for(var j=0;j<size;j++) cell2.push(n);
	return cell2;
}

function getSerieswithnan(data,names){
	var series=[];
	var i=1;
	var size=Object.keys(names).length+1;
	for(var n in names){
		var k=names[n];var d;
		for(var l in data[k]){
			var datacell=data[k][l];
			d=new Date(Number(datacell[0])*MILLIFACTOR);
			cell=[];
			for(var j=0;j<size;j++) cell.push(null);
			cell[0]=d;
			cell[i]=Number(datacell[1]);
			series.push(cell);
		}
		var cell2=getCell(size,null);
		cell2[0]=new Date(d.getTime()+1);
		series.push(cell2);
		i++;
	}
	// sort 
//	series.sort(function(a,b) {return a[0]-b[0];});

	return series;
}

function revertArrayPart(array,index){
	// copy part to reverse
	var copy=[];
	for(var i=array.length-1;i>=index;i--){copy.push(array[i]);}
	for(var i=array.length-1;i>=index;i--){array[i]=copy[i-index];}	
}

function getSerieswithreverse(data,names){ // add local extremum before and after
	// find min and max t
	var tmin=new Date().getTime(), tmax=0;
	var DEC=100;//1*MILLIFACTOR;
	for(var n in names){
		var k=names[n];var d;
		for(var l in data[k]){
			var t=data[k][l][0]*MILLIFACTOR;
			if(t>tmax) tmax=t-DEC;
			if(t<tmin) tmin=t+DEC;
		}
	}
	// before each serie duplicate first point with tmin and last point with tmax
	var series=[];
	var i=1;
	var size=Object.keys(names).length+1;
	var revert=false,lastreverted=0;
	for(var n in names){
		var k=names[n];var d,datacell;
		// check if not already tmin

		var first=true,dt;

		for(var l in data[k]){
			datacell=data[k][l];
			dt=Number(datacell[0])*MILLIFACTOR;
			if(first){
				if(dt>tmin){
					d=new Date(tmin);tmin+=DEC;
					cell=[];
					for(var j=0;j<size;j++) cell.push(null);
					cell[0]=d;
					cell[i]=Number(datacell[1]);
					series.push(cell);
					first=false;					
				} //else {dt=tmin;tmin++;}
			} 
			d=new Date(dt);
			cell=[];
			for(var j=0;j<size;j++) cell.push(null);
			cell[0]=d;
			cell[i]=Number(datacell[1]);
			series.push(cell);
		} 
		if(dt<tmax){
			d=new Date(tmax);tmax-=DEC;
			cell=[];
			for(var j=0;j<size;j++) cell.push(null);
			cell[0]=d;
			cell[i]=Number(datacell[1]);
			series.push(cell);
		} //else series[series.length-1][0]=new Date(tmax--);

//		var cell2=[];
//		for(var j=0;j<size;j++) cell2.push(null);
//		cell2[0]=new Date(d.getTime()+1);
//		series.push(cell2);

		if(revert){
			revertArrayPart(series,lastreverted);
			revert=false;
		} else revert=true;
		lastreverted=series.length;
		i++;
	}
	// sort 
//	series.sort(function(a,b) {return a[0]-b[0];});

	return series;
}


function getSeriesoriginalsorted(data,names){
	var series=[];
	var i=1;

	var size=Object.keys(names).length+1;
	for(var n in names){
		var k=names[n];
		for(var l in data[k]){
			var datacell=data[k][l];
			var d=new Date(Number(datacell[0])*MILLIFACTOR);
			var cell=getSerieByDate(series,d);
			if(!cell) {
				cell=[];
				for(var j=0;j<size;j++) cell.push(null);
				cell[0]=d;
				series.push(cell);
			} 
			cell[i]=Number(datacell[1]);
		}
		i++;
	}
	// sort 
	series.sort(function(a,b) {return a[0]-b[0];});
	return series;
}
  */
/*
 * /*
		var extra2=getSeries(data,seriesnamesy2);
		var labels3=['Date'].concat(seriesnamesy2);
		var height2=30, st2=geid(this.name+"3").style;
		if(st2 && st2.height && st2.height.substr(st2.height.length-2,st2.height.length)=="px") height2=Number(st2.height.substr(0,st2.height.length-2));
		this.rangegraph2=new Dygraph(
				document.getElementById(this.name+"3"),
				extra2,
				{
					xAxisHeight: height2,
					//				labels: ['Date'].concat(config.series.y2),
					//				series: config.series.y2,
					labels: labels3,
					series: series0,
					axes: { 
						drawAxis : false,
						x: {axisLabelFormatter:function(x){return "";}}
					},
					showRangeSelector: true,
					interactionModel: im,
					rangeSelectorHeight: height,
					showLabelsOnHighlight :false,
					rangeSelectorPlotStrokeColor: acolor2,
					rangeSelectorPlotFillGradientColor: acolor2,
					rangeSelectorPlotFillColor:"#FFFFFF"
				}
		);
 */
