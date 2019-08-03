var fn={
		value:"value",
		classfield:"class",
		type:"type",
		max:"max",
		min:"min",
		active:"active",
		timestamp:"timestamp",
		operator:"operator",
		unit:"unit"
};

var datadefs={	
		fieldnames:fn,
		byclassfullorder:{
		"input":{main:[fn.value,fn.min, fn.max], detail:[fn.timestamp]},
		"Comparator":{main:[fn.active,"lefthand", "operator", "righthand","conditionmet"], detail:["src","dst"]},
		"Timer":{main:[fn.active,"step", "cyc", "dur"],detail:[,"src", "dst",fn.timestamp]}
		},
		byfieldorder:{
			overview:[fn.active, fn.value],
			detail:[fn.min,fn.max],
			superdetail:[fn.classfield,fn.type,fn.unit],
			seq:{"src":"dst"}	// fields that must appear in sequence			
		},
		fieldformat:{
			symbolsnamepattern:[fn.operator],
			boolnamepattern:[fn.active,"cyc","conditionmet"],
			timestampnamepattern:[fn.timestamp]
		},
		valueformat:{
			symbols:{"greaterorequal":">=","less":"<"}
		},
		concattab:{}		// fields that must be concatened in value
};
datadefs.concattab[fn.value]=fn.unit;
datadefs.concattab[fn.min]=fn.unit;


if(!globalThis.globaldefs) globalThis.globaldefs={};
 globalThis.globaldefs.data=datadefs;