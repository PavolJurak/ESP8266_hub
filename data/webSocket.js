var ws = new WebSocket("ws://"+location.hostname+":81/",["arduino"]);

ws.onopen = function(){
	ws.send('Connect ' + new Date());
};

ws.onerror = function (error) {
    console.log('WebSocket Error ', error);
};

ws.onmessage = function (e) {  
    console.log('Server: ', e.data);
};

ws.onclose = function(){
    console.log('WebSocket connection closed');
};

function sendRGB(){
	var r = document.getElementById('r').value;
    var g = document.getElementById('g').value;
    var b = document.getElementById('b').value;
	
	var rgb = r << 16 | g << 8 | b;
	var rgbstr = '#'+ rgb.toString(16);
	console.log('RGB: ' + rgbstr);
    ws.send(rgbstr);
}