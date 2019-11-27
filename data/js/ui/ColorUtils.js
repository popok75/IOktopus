 export {generateColors, mixColors};

function generateColors(size, colorset){
	var cthreshold=40;
	if(!colorset) colorset=['#4444AA','#AA4444','#44AA44','#AAAA44'];


	for(var i=colorset.length;i<size;i++){
		var newcol,ok=false, tries=100;
		while(!ok){
			tries--;
			if(tries<0) {
				tries=100;
				cthreshold--;
			//	console.log("reducing color threshold to "+cthreshold);
			}
			newcol=getRandomColor();
			ok=true;
			for(var col in colorset){
				var delta=deltaE(hexToRgb(newcol),hexToRgb(colorset[col]));
				//	console.log("color diff between "+newcol+" & "+col+":"+this.colorset[col]+" -> "+delta);
				if(delta<cthreshold) {ok=false; break;}
			}
		}
		colorset.push(newcol);
	}
	return colorset;
};
 
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
  



function mixColors(colarray){
	/*!
	 * Color_mixer for substractive color mixing
	 * requires jQuery and jQuery color
	 * Author: Andy Soiron
	 * http://www.andysoiron.de
	 */

	var Color_mixer = {};

	Color_mixer.toCymk = function(color){
		var cyan    = 255 - color.r;
		var  magenta = 255 - color.g;
		var yellow  = 255 - color.b;
		var black   = Math.min(cyan, magenta, yellow);
		if(black==255) return {c:0,m:0,y:0,k:1,a:color.a};
		var cyan    = ((cyan - black) / (255 - black));
		var  magenta = ((magenta - black) / (255 - black));
		var  yellow  = ((yellow  - black) / (255 - black));

		return {c:cyan,m:magenta,y:yellow,k:black/255,a:color.a};
	}

	Color_mixer.toRgba = function(color){
		color.c = color.c;
		color.m = color.m;
		color.y = color.y;
		color.k = color.k;
		var R = color.c * (1.0 - color.k) + color.k;
		var G = color.m * (1.0 - color.k) + color.k;
		var  B = color.y * (1.0 - color.k) + color.k;
		R = Math.round((1.0 - R) * 255.0 + 0.5);
		G = Math.round((1.0 - G) * 255.0 + 0.5);
		B = Math.round((1.0 - B) * 255.0 + 0.5);
		color = {r:R,g:G,b:B,a:color.a};
		return color;
	}

	Color_mixer.mix = function(color1,color2){
		if(typeof(color1)=='object'&&(color1 instanceof Array)==false)
			color1 = new Array(color1,color2);

		var C = 0, M = 0, Y = 0, K = 0, A = 0;
		for(var i=0;i<color1.length;i++){
			color1[i] = Color_mixer.toCymk(color1[i]);
			C += color1[i].c;
			M += color1[i].m;
			Y += color1[i].y;
			K += color1[i].k;
			A += color1[i].a;
		}
		C = C/color1.length;
		M = M/color1.length;
		Y = Y/color1.length;
		K = K/color1.length;
		A = A/color1.length;
		var color = {c:C,m:M,y:Y,k:K,a:A};
		color = Color_mixer.toRgba(color);
		return color;
	}
	function hexToRgb(hex) {
		var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
		return result ? {
			r: parseInt(result[1], 16),
			g: parseInt(result[2], 16),
			b: parseInt(result[3], 16),
			a:255
		} : null;
	}
	function componentToHex(c) {
		var hex = c.toString(16);
		return hex.length == 1 ? "0" + hex : hex;
	}

	function rgbToHex(c) {
		return "#" + componentToHex(c.r) + componentToHex(c.g) + componentToHex(c.b);
	}

	function simplemix(colset){
		var r=0,g=0,b=0,a=0;
		for(var k in colset){
			r+=colset[k].r;
			g+=colset[k].g;
			b+=colset[k].b;
			a+=colset[k].a;
		}
		r=r/colset.length;
		g=b/colset.length;
		b=b/colset.length;
		a=a/colset.length;
		return {r:r,g:g,b:b,a:a};
	}

	var ca2=[];
	for(var k in colarray) ca2[k]=hexToRgb(colarray[k]);
//	return rgbToHex( simplemix(ca2));
	return  rgbToHex(Color_mixer.mix(ca2));
}
  
