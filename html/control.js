$(document).ready(() => {
    connectWebSocket().then(() => assignButtons());
});


var webSocket;
var pingTimer;
var mouseTimer;

function sendWhileHolding(id, downVal, upVal){
    $(id).on("mousedown", () => {
        webSocket.send(downVal);
        mouseTimer = setInterval(() => webSocket.send(downVal), 30);
    }).on("mouseup mouseleave", () => {
        clearTimeout(mouseTimer);
        webSocket.send(upVal);
    });
}

function assignButtons(){
    sendWhileHolding("#up", "U", "X");
    sendWhileHolding("#down", "D", "X");
    sendWhileHolding("#left", "L", "X");
    sendWhileHolding("#right", "R", "X");
}

function connectWebSocket() {
  var deferred = new $.Deferred();
  
  infoMessage("Socket connecting");
  
  webSocket = new WebSocket(`ws://${window.location.host}/control.ws`);
  
  webSocket.onopen = (evt) => {
    deferred.resolve(webSocket);
    ping();
    successMessage("Socket opened").then(() => clearMessage());
  };

  webSocket.onclose = (evt) => {
    if(evt.wasClean){
        warningMessage("Socket closed");
    }
    else {
        errorMessage("Socket error");
    }
    setTimeout(() => connectWebSocket(), 10000);
  };

  webSocket.onerror = (evt) => {
    // NOTHING
  };
  
  webSocket.onmessage = (evt) => {
    if(evt.data == "pong"){
        ping();
    }
  };
  
  return deferred.promise();
}

function ping(){
    clearTimeout(pingTimer);
    if (webSocket != 0 && webSocket.readyState == 1){
        setTimeout(() => {
            webSocket.send("ping");
            pingTimer = setTimeout(() => webSocket.close(), 250);
        }, 2000);
    }
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