window.addEventListener('load', () => {
	console.log("Hello world");

	var button = document.getElementsByTagName('button')[0];
	var input = document.getElementsByTagName('input')[0];
 	var output = document.getElementsByTagName('div')[0];

	var txCharacteristic;

	var NORDIC_SERVICE = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
	var NORDIC_TX = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
	var NORDIC_RX = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

	var filters = [];
	var services = [];

	filters.push({namePrefix: "UART"});
	services.push(NORDIC_SERVICE);

	function connectionDisconnectCallback(){

	}

	function openCallback(){

	}

	function receiveCallback(){

	}

	function ab2str(buf) {
	  return String.fromCharCode.apply(null, new Uint8Array(buf));
	}

	function str2ab(str) {
	  var buf = new ArrayBuffer(str.length);
	  var bufView = new Uint8Array(buf);
	  for (var i=0, strLen=str.length; i<strLen; i++) {
	    bufView[i] = str.charCodeAt(i);
	  }
	  return buf;
	}

	input.addEventListener("keyup", function(event) {
		event.preventDefault();
		if (event.keyCode === 13) {
			if (txCharacteristic){
				console.log("HT> Sending "+ input.value);
				txCharacteristic.writeValue(str2ab(input.value)).then(function() {
					console.log("HT> Sent");
					input.value = "";
				});
			}
		}
	});

	button.addEventListener('click', () => {
		navigator.bluetooth.requestDevice({
			filters: filters,
		  optionalServices: services}).then(function(device) {

				deviceName = device.name;
				console.log('HT>  Device Name:       ' + device.name);
				console.log('HT>  Device ID:         ' + device.id);
				device.addEventListener('gattserverdisconnected', function() {
					console.log("HT> Disconnected (gattserverdisconnected)");
					closeSerial();
				}, {once:true});
				return device.gatt.connect();
			}).then(function(server) {
				console.log("HT> Connected");
				btServer = server;
				return server.getPrimaryService(NORDIC_SERVICE);
			}).then(function(service) {
				console.log("HT> Got service");
				btService = service;
				return btService.getCharacteristic(NORDIC_RX);
			}).then(function (characteristic) {
				rxCharacteristic = characteristic;
				console.log("HT> RX characteristic:"+JSON.stringify(rxCharacteristic));
				rxCharacteristic.addEventListener('characteristicvaluechanged', function(event) {
		        var value = event.target.value.buffer;
		        console.log("HT> " +  event.timeStamp + " RX:"+ab2str(value));
		        output.innerHTML += "<p>" + ab2str(value) + "</p>";
		        receiveCallback({"timeStamp": event.timeStamp, "value": value});
		      });
				return rxCharacteristic.startNotifications();
			}).then(function() {
				return btService.getCharacteristic(NORDIC_TX);
			}).then(function (characteristic) {
				txCharacteristic = characteristic;
				console.log("BT> TX characteristic:"+JSON.stringify(txCharacteristic));
    	}).catch(function(error) {
    	console.log('BT> ERROR: ' + error);
    	if (connectionDisconnectCallback) {
    		connectionDisconnectCallback(undefined);
    		connectionDisconnectCallback = undefined;
    	}
    });
		})
	})
