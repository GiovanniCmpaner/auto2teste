$(document).ready(() => {
    connectWebSocket().then(() => assignButtons());
});


var wsControl;
var mouseTimer;

function sendWhileHolding(id, downVal, upVal){
    $(id).on("mousedown", () => {
        wsControl.send(downVal);
        clearTimeout(mouseTimer);
        mouseTimer = setInterval(() => wsControl.send(downVal), 30);
    }).on("mouseup mouseleave", () => {
        //clearTimeout(mouseTimer);
        //wsControl.send(upVal);
    });
}

function assignButtons(){
    sendWhileHolding("#up", "U", "X");
    sendWhileHolding("#down", "D", "X");
    sendWhileHolding("#left", "L", "X");
    sendWhileHolding("#right", "R", "X");
    sendWhileHolding("#cross", "X");
}

function connectWebSocket() {
  var deferred = new $.Deferred();
  
  infoMessage("Socket connecting");
  
  wsControl = new WebSocket(`ws://${window.location.host}/control.ws`);
  
  wsControl.onopen = (evt) => {
    deferred.resolve(wsControl);
    successMessage("Socket opened").then(() => clearMessage());
  };

  wsControl.onclose = (evt) => {
    if(evt.wasClean){
        warningMessage("Socket closed");
    }
    else {
        errorMessage("Socket error");
    }
    setTimeout(() => connectWebSocket(), 10000);
  };

  wsControl.onerror = (evt) => {
    // NOTHING
  };
  
  wsControl.onmessage = (evt) => {
    // NOTHING
  };
  
  return deferred.promise();
}

function clearMessage() {
    if (typeof this.fadeOutHandle != "undefined") {
        clearTimeout(this.fadeOutHandle);
    }
    this.fadeOutHandle = setTimeout(() => $("#message").fadeOut(250), 2000);
}

function infoMessage(text) {
    return $("#message").prop("class", "info").text(text).fadeTo(250, 1.0).promise();
}

function successMessage(text) {
    return $("#message").prop("class", "success").text(text).fadeTo(250, 1.0).promise();
}

function warningMessage(text) {
    return $("#message").prop("class", "warning").text(text).fadeTo(250, 1.0).promise();
}

function errorMessage(text) {
    return $("#message").prop("class", "error").text(text).fadeTo(250, 1.0).promise();
}