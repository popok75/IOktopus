


class UserView  extends EventEmitter {
	constructor(){
		super();
		console.log("UserView");
		this.panel=new Panel();
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

		var nview= new NodeView(name,data);
		subgrouppanel.addComponent(nview.getComponent());

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
	
	createLogView(data,nodes){
		var tooshort=true;
		for(var j in data) if(data[j].length>1) tooshort=false;
		if(tooshort) return;
		this.initLogView(data,nodes);
	}	
	
	initLogView(data,nodes){
		this.logview= new LogGraphicView(data,nodes);	
		this.panel.addComponent(this.logview.getComponent());
		var loghtml=this.logview.getComponent().getHtml();	
		geid('loginterface').innerHTML=loghtml;	//should not be added like that
		this.logview.init();
	}

	updateLogView(data){
		if(!data || data.length==0 || data[0].length==0) {console.log("Log data table received from /log/ empty.");return;}
		if(!this.logview) this.initLogView(data);
		this.logview.update(data);
	}

	init(){
		var html=this.panel.getHtml()
		geid('userinterface').innerHTML=html;
	}

	update(){

	}
};






///////////////
class NodeView extends EventEmitter{
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



var lightred="#FFDDDD"

function getNumber(text){
	while(text.search(" ")>=0) text=text.replace(" ", "");
	if(!text.length || isNaN(text)) return Number.NaN;
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
		
		var textcomp=new TextAreaComponent(this.name+"text","",1.5);
		
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
/*	HTML 5 Prevent autoplay
function stopSound(id){
	var item=document.getElementById(id)
	if(item) {item.pause();//item.remove();
	}
}

//var audiosample = new Audio("bell.mp3");

function playSound(url,id, loop){
	  var src=url;
	//  var item=geid(id);
	 // if(item) src=item.getAttribute('data-url');
	
	  var audio = document.createElement('audio');
	  audio.id=id;
	  audio.style.display = "none";
	  audio.src = url;
	  audio.url = url;
	  audio.loop=loop;
	  audio.autoplay = true;
	  audio.onended = function(){
	  //  audio.remove(); //Remove when played.
	  };
	  document.body.appendChild(audio);
	}
*/

///////////////
///////////////
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
class LogGraphicView {
	constructor(data,nodes){
		this.name="graphiclog";
		this.rawdata=data;
		this.nodes=nodes;
		this.component= new Panel(this.name+"container","","log ("+(data.length-1)+" time point)");
		this.graphgroup=new LogGraphGroup(this.name,this.component,nodes);
	}

	init(){
		this.addGraph(this.rawdata);
	}

	addGraph(data){ 
		this.graphgroup.init(data,this.nodes);
	}

	update(data){
		geid(this.name+"container").children[0].children[0].innerHTML="log ("+(data.length-1)+" time point)";// should move this to panel.setlegend
		this.graphgroup.update(data);	// he is not supposed to change data object !!!
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










