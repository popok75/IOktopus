 function globalexport(obj){for(var k in obj) {globalThis[k]=obj[k];}};
globalThis.globalexport=globalexport;