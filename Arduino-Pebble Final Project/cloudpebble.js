
var comfortModeToggle = 0;
var setTempInt = 23; //default temp
var LOWER_BOUND = 20;
var UPPER_BOUND = 23;
var settingTemp = false;
var inCMode = false;
var printUpstairs = false;
var serverPrintMode = false;
var standbyMode = 0;
var printPooling = true;
var connected = false;

Pebble.addEventListener("appmessage",
  function(e) {
    if (e.payload) {
      if (e.payload.name) sendToServer(0); //get new reading
    }
    else Pebble.sendAppMessage({ "0": "nopayload" });  
  }
);

Pebble.addEventListener("appmessage",
  function(e) {
    if (e.payload) {
      if (e.payload.polling) {
        Pebble.sendAppMessage({ "0": "Polling Mode ON" });
      }
    }
    else Pebble.sendAppMessage({ "0": "nopayload" });  
  }
);

Pebble.addEventListener("appmessage",
  function(e) {
    if (e.payload) {
      if (e.payload.pollingOff) {
        Pebble.sendAppMessage({ "0": "Polling Mode OFF" });
      }
    }
    else Pebble.sendAppMessage({ "0": "nopayload" });  
  }
);

Pebble.addEventListener("appmessage", 
  function(e) {
    if (e.payload) {
      if (e.payload.changeUnit) {
        serverPrintMode = true;
        sendToServer(1); //change unit
      }
      //else Pebble.sendAppMessage({ "0": "problemChangeUnit/down" }); 
  }
  }
);

Pebble.addEventListener("appmessage", 
  function(e) {
    if (e.payload) {
      if (e.payload.getStats) {
        serverPrintMode = true;
        sendToServer(4); //get stats
      }
      //else Pebble.sendAppMessage({ "0": "problemChangeUnit/down" }); 
  }
  }
);


Pebble.addEventListener("appmessage", 
  function(e) {
    if (e.payload) {
      if (e.payload.partyMode) {
          serverPrintMode = true;
          sendToServer(3); //partymode toggled 
      }
      //else Pebble.sendAppMessage({ "0": "problemChangeUnit/down" }); 
    }
  }
);


//***********************Comfort-mode-stuff begins**************************

Pebble.addEventListener("appmessage", 
  function(e) {
    if (e.payload) {  
      if (e.payload.cMode) {;
        comfortModeToggle++;
        if (comfortModeToggle % 2 == 1) {
          settingTemp = true;
          inCMode = true;
          Pebble.sendAppMessage({ "0": "Comfort-Mode ON" });
        } 
        else {
          UPPER_BOUND = setTempInt;
          settingTemp = false;
          Pebble.sendAppMessage({ "0": "Done Setting threshold." });
        }
      }
      //else Pebble.sendAppMessage({ "0": "problemChangeUnit/down" }); 
  }
  }
);


Pebble.addEventListener("appmessage", 
  function(e) {
    if (e.payload) {
      if (e.payload.cModeExit) {
          settingTemp = false;
        inCMode = false;
          Pebble.sendAppMessage({ "0": "Comfort-Mode OFF" });
      }
      //else Pebble.sendAppMessage({ "0": "problemChangeUnit/down" }); 
  }
  }
);

Pebble.addEventListener("appmessage", 
  function(e) {
    if (e.payload) {
      if (e.payload.setTempUp) {
        if (settingTemp) {
          setTempInt++;
          var sendMe = "Double-click up/down to change by one degree\n" + setTempInt.toString();
          Pebble.sendAppMessage({ "0": sendMe});
        }
      }
      //else Pebble.sendAppMessage({ "0": "problemChangeUnit/down" }); 
  }
  }
);

Pebble.addEventListener("appmessage", 
  function(e) {
    if (e.payload) {
      if (e.payload.setTempDown) {
        if (settingTemp) {
          setTempInt--;
          var sendMe = "Double-click up/down to change by one degree\n" + setTempInt.toString();
          Pebble.sendAppMessage({ "0": sendMe});
        }
      }
  }
  }
);
//****************************Comfort-mode-stuff ends*****************************88



Pebble.addEventListener("appmessage", 
  function(e) {
    if (e.payload) {
      if (e.payload.standby) {
          serverPrintMode = true;
          sendToServer(2);
      }
      //else Pebble.sendAppMessage({ "0": "problemStandBy/up" }); 
  }
  }
);


Pebble.addEventListener("appmessage", 
  function(e) {
    if (e.payload) {
      if (e.payload.stopServer) {
          Pebble.sendAppMessage({ "0": "Server Shut-Down"});
          sendToServer(5);
      }
      //else Pebble.sendAppMessage({ "0": "problemStandBy/up" }); 
  }
  }
);

function sendToServer(state) {
  //var connected = false;
    var req = new XMLHttpRequest();
    var ipAddress = "158.130.63.14"; // Hard coded IP address
    var port = "3001"; // Same port specified as argument to server
    var url = "http://" + ipAddress + ":" + port + "/";
  //var method = "TEST:BACK TO START";
    var method = "GET";
  var async = true;
    req.onload = function(e) {
    connected = true;
                // see what came back
                var msg = "no response";
                var response = JSON.parse(req.responseText);
                if (response) {
                    if (response.name) {
                        msg = response.name;
                    }
                  else {
                    msg = "Can't Connect";
                  Pebble.sendAppMessage({ "0": msg });
                  }
                }
    // sends message back to pebble
    if (serverPrintMode) {
      if (msg.charAt(0) == '&') msg = msg.substring(1);
          Pebble.sendAppMessage({ "0": msg });
          serverPrintMode = false;
        }
    else {
      var ForC = msg.charAt(0);
      msg = msg.substring(1);
      var temp = parseInt(msg);
      if (ForC === '&') {
          Pebble.sendAppMessage({ "0": msg });
      }
      else if(printUpstairs) {  
          printUpstairs = false;
      }
      else {
          msg = "Current Temperature\n" + msg + " " + ForC;
          if(inCMode)  {if (temp > UPPER_BOUND && ForC === 'C') msg = msg + " TOO HOT";}
          //if(inCMode) {if (temp < LOWER_BOUND && ForC === 'C') msg = msg + " TOO COLD"}
          Pebble.sendAppMessage({ "0": msg });
      }
    } 
  }
  if (!connected) Pebble.sendAppMessage({ "0": "Can't Connect to Server" })
  connected = false; //reset connected
  req.open(method, url, async);
  
  //req.send(null);
  //req.setRequestHeader("Content-Type", "text/plain;charset=UTF-8");
  var sendString = "$" + state;
  req.setRequestHeader("Content-Type", sendString);
  req.send(); //sends "change color" codes to server
  //if (!connected) Pebble.sendAppMessage({ "0": "Can't connect" });
}