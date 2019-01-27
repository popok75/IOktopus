


//Notes : - this file should contain only generic logdygraph
//- code could be improved



var MILLIFACTOR=1000;	//ts in seconds not in millis
var ONEDOTMSAROUND=30000;
var USECSV=false;
var USERANGESUBSET=true;


class LogGraphGroup {	//interface the data and the graphs
	constructor(name,component,nodes){

		this.name=name;
		this.graphs=[];
		this.component=component;
		var graphid=0;
		var cthreshold=40;
		this.colorset=['#4444AA','#AA4444','#44AA44','#AAAA44'];
		var size=Object.keys(nodes).length;
		for(var i=4;i<size;i++){
			var newcol,ok=false, tries=100;
			while(!ok){
				tries--;
				if(tries<0) {tries=100;cthreshold--;console.log("reducing color threshold to "+cthreshold);}
				newcol=getRandomColor();
				ok=true;
				for(var col in this.colorset){
					var delta=deltaE(hexToRgb(newcol),hexToRgb(this.colorset[col]));
					//	console.log("color diff between "+newcol+" & "+col+":"+this.colorset[col]+" -> "+delta);
					if(delta<cthreshold) {
						ok=false; break;
					}
				}
			}
			this.colorset.push(newcol);
		}

		this.createCanvas(this.name+graphid);
		this.createConfig(nodes);
		var graphid=0;
		graphid++;
		for(var i in nodes) {
			this.createRangeCanvas(this.name+graphid);
			graphid++;
		}		
	}


	createConfig(nodes){
		// group by unit
		var groups={};
		for(var i in nodes) {
			var u=nodes[i]["unit"];
			if(u==undefined) u="";
			if(!groups[u]) groups[u]=[];
			groups[u].push(i);
		}

		var config={};
		config["series"]={y:[],y2:[]};
		config["axis"]={y:{},y2:{}};
		var c=0,op=0;
		for(var i in groups) {
			if (op==0) {
				for(var gm in groups[i]) {config.series.y.push({name:groups[i][gm],unit:i,color:this.colorset[c]});c++;}
				config.axis.y={unit:i};
			}
			else if(op==1) {
				for(var gm in groups[i]) {config.series.y2.push({name:groups[i][gm],unit:i,color:this.colorset[c]});c++;}
				config.axis.y2={unit:i};

			}
			else break;
			op++;
		} // if more than 2 groups should create a new graph
		this.groups=groups;
		
		
		this.config=config;

	}

	init(rawdata,labels,nodes){
		
		this.config.labels=labels;
		
		var graphid=0;
		var graph=new LogDygraph(this.name+graphid);// who create the canvas tag ?
		var data=rawdata;//this. prepareData(rawdata);
		graph.init(data,this.config);
		graphid++;
		// create a range per variable
		for(var i in this.groups) {
			for(var j in this.groups[i]) {
				graph.addRange(this.name+graphid,this.groups[i][j], this.colorset[graphid-1]);	// add a range slider for each group
				graphid++;			    
			}
		}

		this.graph=graph;

	};

	update(rawdata,labels){
		//var data=this. prepareData(rawdata);
		this.graph.update(rawdata,labels);
	};

	createCanvas(canvasname){
		var component= new Component(canvasname);
		component.tag="div";
		component.style="width:100%;"//height:200px;";	 
			this.component.addComponent(component);

	}
	createRangeCanvas(canvasname){
		var component2= new Component(canvasname);
		component2.tag="div";
		component2.style="width:100%;height:30px"; 
		this.component.addComponent(component2);

	}


}
/*	var config={
series:{
	y:[{name:"Humidity",unit:"%",color:colorset[0]}],
	y2:[{name:"Temperature",unit:"\u00B0C",color:colorset[1]}],
},			
axis:{
	y:{color:colorset[0],unit:"%",label:"RH%"},
	y2:{color:colorset[1],unit:"\u00B0C",label:"T\u00B0C"},

}
};*/


function getRandomColor() {
	var letters = '0123456789ABCDEF';
	var color = '#';
	for (var i = 0; i < 6; i++) {
		color += letters[Math.floor(random() * 16)];
	}
	return color;
}

var seed = 1;
function random() {
	var x = Math.sin(seed++) * 10000;
	return x - Math.floor(x);
}

function deltaE(rgbA, rgbB) {
	let labA = rgb2lab(rgbA);
	let labB = rgb2lab(rgbB);
	let deltaL = labA[0] - labB[0];
	let deltaA = labA[1] - labB[1];
	let deltaB = labA[2] - labB[2];
	let c1 = Math.sqrt(labA[1] * labA[1] + labA[2] * labA[2]);
	let c2 = Math.sqrt(labB[1] * labB[1] + labB[2] * labB[2]);
	let deltaC = c1 - c2;
	let deltaH = deltaA * deltaA + deltaB * deltaB - deltaC * deltaC;
	deltaH = deltaH < 0 ? 0 : Math.sqrt(deltaH);
	let sc = 1.0 + 0.045 * c1;
	let sh = 1.0 + 0.015 * c1;
	let deltaLKlsl = deltaL / (1.0);
	let deltaCkcsc = deltaC / (sc);
	let deltaHkhsh = deltaH / (sh);
	let i = deltaLKlsl * deltaLKlsl + deltaCkcsc * deltaCkcsc + deltaHkhsh * deltaHkhsh;
	return i < 0 ? 0 : Math.sqrt(i);
}

function rgb2lab(rgb){
	let r = rgb[0] / 255, g = rgb[1] / 255, b = rgb[2] / 255, x, y, z;
	r = (r > 0.04045) ? Math.pow((r + 0.055) / 1.055, 2.4) : r / 12.92;
	g = (g > 0.04045) ? Math.pow((g + 0.055) / 1.055, 2.4) : g / 12.92;
	b = (b > 0.04045) ? Math.pow((b + 0.055) / 1.055, 2.4) : b / 12.92;
	x = (r * 0.4124 + g * 0.3576 + b * 0.1805) / 0.95047;
	y = (r * 0.2126 + g * 0.7152 + b * 0.0722) / 1.00000;
	z = (r * 0.0193 + g * 0.1192 + b * 0.9505) / 1.08883;
	x = (x > 0.008856) ? Math.pow(x, 1/3) : (7.787 * x) + 16/116;
	y = (y > 0.008856) ? Math.pow(y, 1/3) : (7.787 * y) + 16/116;
	z = (z > 0.008856) ? Math.pow(z, 1/3) : (7.787 * z) + 16/116;
	return [(116 * y) - 16, 500 * (x - y), 200 * (y - z)]
}
/*//	To use it, just pass in two rgb arrays:
	deltaE([128, 0, 255], [128, 0, 255]); // 0
	deltaE([128, 0, 255], [128, 0, 230]); // 3.175
	deltaE([128, 0, 255], [128, 0, 230]); // 21.434
	deltaE([0, 0, 255], [255, 0, 0]); // 61.24
 */ 
function hexToRgb(hex) {
	var m = hex.match(/^#?([\da-f]{2})([\da-f]{2})([\da-f]{2})$/i);
	var arr= [parseInt(m[1], 16), parseInt(m[2], 16), parseInt(m[3], 16)];
	return arr;

}





class LogDygraph {
	constructor(name){
		this.name=name;
		this.ranges=[];
		this.usecsv=USECSV;
		this.userangesubset=USERANGESUBSET;
	}


	getMainSeries(){
		if(this.usecvs)return toCSV( this.data);
		return this.data;
	}

	init(data,config)
	{	
		this.config=config;
		this.data=data;
		this.labels=config.labels;

		var dataseries=this.getMainSeries();
		var yunit="",y2unit="";
		if(config.axis){
			if(config.axis.y2 && config.axis.y2.color) changeStylesheetRule(document.styleSheets[0], ".dygraph-axis-label-y2","color",config.axis.y2.color);
			if(config.axis.y && config.axis.y.color) changeStylesheetRule(document.styleSheets[0], ".dygraph-axis-label-y","color",config.axis.y.color);
			if(config.axis.y2 && config.axis.y2.unit) y2unit=config.axis.y2.unit;
			if(config.axis.y && config.axis.y.unit) yunit=config.axis.y.unit;
		}
		//	changeStylesheetRule(document.styleSheets[0], ".dygraph-legend","text-align","right");
		changeStylesheetRule(document.styleSheets[0], ".dygraph-legend","text-align","left");
		changeStylesheetRule(document.styleSheets[0], ".dygraph-legend","margin-left","80px");
//		changeStylesheetRule(document.styleSheets[0], ".dygraph-legend","z-index","99 !important");
		//	background: transparent !important;
		//	changeStylesheetRule(document.styleSheets[0], ".dygraph-legend","width","200px");

		var vunit="",v2unit=""
			if(config.series.y[0].unit) vunit=config.series.y[0].unit;
		if(config.series.y2[0] && config.series.y2[0].unit) v2unit=config.series.y2[0].unit;
		var vf1=new labelformatter(vunit,yunit);	//for multiple on one axis, should find a unit per line
		var vf2=new labelformatter(v2unit,y2unit);
		
		//var labels0=['Date'];
		var labels0=config.labels;
		//var seriesnamesy=getPropertySubArray(this.config.series.y,"name");
		//var seriesnamesy2=getPropertySubArray(this.config.series.y2,"name");
		//labels0=labels0.concat(seriesnamesy.concat(seriesnamesy2));
		var series0={};
		for(var n in config.series.y) {series0[config.series.y[n].name]={axis: 'y'};}
		for(var n in config.series.y2) {series0[config.series.y2[n].name]={axis: 'y2'};}
		var title0= config.title | " ";

		this.im={'mousedown' : downV3,'mousemove' : moveV3,'mouseup' : upV3,'click' : clickV3,'dblclick' : dblClickV3,
				'mousewheel' : function(event, g, context){	scrollV3(event, g, context);}};


		var vcolorsy=getPropertySubArray(config.series.y,"color");
		var vcolorsy2=getPropertySubArray(config.series.y2,"color");
		var vcolorsall=vcolorsy.concat(vcolorsy2);
		this.vcolorsall=vcolorsall;
		Dygraph.ANIMATION_DURATION=0;

		function legendFormatter(data) {
			var sorted=data.series.sort(function(ao,bo){
				var a=ao.label,b=bo.label;
				if(a<b) return -1;
				if(a==b) return 0;
				if(a>b) return 1;
			});
			if (data.x == null) {
				// This happens when there's no selection and {legend: 'always'} is set.

				var legend= '' + sorted.map(function(series) {return '<span style="font-weight: bold; color: '+series.color+';">'+series.dashHTML +  series.labelHTML+'</span>' }).join('<br>');
				return legend;
			}

			var html = this.getLabels()[0] + ': ' + data.xHTML;

			sorted.forEach(function(series) {
				var val=series.yHTML;
				if(val==undefined) val="-";	
				if (!series.isVisible) return;
				var labeledData = '<span style="text-align: left; color: '+series.color+';">'+series.dashHTML +'&nbsp;'+  series.labelHTML + ': ' + val+'</span>';
				if (series.isHighlighted) {
					labeledData = '<b>' + labeledData + '</b>';
				}
				html += '<br>' + series.dashHTML + ' ' + labeledData;
			});
			return html; 
			//	return data;
		}

		var opt={
				underlayCallback: (context) => {
					context.save();
				},
				drawCallback: (dygraph) => {
					dygraph.canvas_ctx_.restore();
				},
				labels: labels0,
				animatedZooms: false,
				//visibility: [false, false,false,false,false, false,false,false],
				series: series0,
				//      customBars: true,
				title: title0,
				pointSize: 1.5,
				connectSeparatedPoints: true,
				drawPoints: true,
				colors: vcolorsall,
				ylabel: config.axis.y.label | "", y2label: config.axis.y2.label | "",
				labelsSeparateLines: true,
				labelsShowZeroValues: false,
				highlightSeriesBackgroundAlpha : 0.8,
				highlightSeriesOpts: { 
					strokeWidth: 2,
					strokeBorderWidth: 1,
					highlightCircleSize: 4 },
					legendFormatter: legendFormatter,
					axes: { y: vf1, 
						y2: vf2, 
						axisLabelWidth: 100
					},
					legend: 'always',//showRangeSelector: true,
					interactionModel: this.im,
					showRoller: true//, errorBars: true
		};
		var min =new Date(dataseries[0][0]).getTime()-ONEDOTMSAROUND,max=new Date(dataseries[0][0]).getTime()+ONEDOTMSAROUND;
		if(min<0) min=0;
		if(dataseries.length==1) opt.dateWindow=[new Date (min),new Date(max)];

		 

		this.maingraph=new Dygraph(
				geid(this.name),
				dataseries,
				opt
		); 
	//	this.series0=series0;
	//	this.labels0=labels0;

	};

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

	getRangeSeries(rangename){
		if(!this.userangesubset) return this.getMainSeries();
		// possibly reduce data to one per axis (including the showed data) to improve performance ?
		var i=0;
		for(;i<this.labels.length;i++) if(this.labels[i]==rangename) break;
		if(i==this.labels.length) return 0;
		var series= getSubSeries(this.data, i);
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
			if(seriesnamesy2) labels0.push(seriesnamesy2[0]);
			return labels0;

		} else return this.labels;
	}
	getRangeSeriesObj(name,seriesnamesy, seriesnamesy2){ 
		var series0={};
		for(var n in seriesnamesy) {series0[seriesnamesy]={axis: 'y'};}
		for(var n in seriesnamesy2) {series0[seriesnamesy2]={axis: 'y2'};}	
		series0[name].showInRangeSelector = true;
		return series0;
	}

	////////////////////////////////
	addRange(rangename, name,color){
		// add minmax as fake data to force valueRange for rangeselector but add a visual glitch when too few data, is it a todo ?

		var dataseries=this.getRangeSeries(name);
		var seriesnamesy=this.getRangeNamesY(name);
		var seriesnamesy2=this.getRangeNamesY2(name);
		var config=this.config;
		var rangeid=this.ranges.length+1;
		var labels0=this.getRangeLabels(name, seriesnamesy2);
		var series0=this.getRangeSeriesObj(name,seriesnamesy, seriesnamesy2);

		var height=30, st=geid(rangename).style;
		if(st && st.height && st.height.substr(st.height.length-2,st.height.length)=="px") height=Number(st.height.substr(0,st.height.length-2));



		var rangegraph1=new Dygraph(
				geid(rangename),
				dataseries,
				{
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
				}
		);

		this.ranges.push(rangegraph1);
		var all=[];
		all.push(this.maingraph);
		for(var r in this.ranges) all.push(this.ranges[r]);
		if(all.length>1) Dygraph.synchronize(all, {selection: false,zoom: true,range: false});

	}


	updateMainGraph(){
		var dataseries=this.getMainSeries();
		var opt={ 'file': dataseries } ;
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
		*/
		
		
		// update
		this.maingraph.updateOptions( opt,false);
	}

	updateRangeGraph(rangegraph,r){
		var rangename=rangegraph.getLabels()[1];
		var dataseries=this.getRangeSeries(rangename);
		var opt={ 'file': dataseries } ;
		// keep now visible
		var max=dataseries[dataseries.length-1][0].getTime();
		/*
		if(this.maingraph.xAxisRange()[1]>=this.maingraph.xAxisExtremes()[1] &&  // most recent was visible untill now
				this.maingraph.xAxisRange()[1] < max)	// and it would disappear 	
			opt.dateWindow=[this.maingraph.xAxisRange()[0],max];	//adjust view
			*/
		opt.dateWindow=[this.maingraph.xAxisRange()[0],this.maingraph.xAxisRange()[1]];
		// update
		rangegraph.updateOptions( opt);
	}

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
		 */	}
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

function labelformatter(axisunit){ 
//	this.unit=unit;
//	this.axesunit=axisunit;
	this.valueFormatter= function(y) {
		var str=y.toFixed(2);
		if(axisunit) str+= axisunit;
		return str; 
	}.bind(this);
	this.axisLabelFormatter= function(y) {
		var str=y.toFixed(2);
		if(axisunit) str+= axisunit;
		return str; 
	}.bind(this);
	this.pixelsPerLabel =20 ;
};

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
 */




function getSubSeries(data,column){
	var series=[];
	for(var i in data) {
		if(data[i][column]!=null) series.push([data[i][0],data[i][column],0]);
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
