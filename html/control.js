$(document).ready(() => {
    connectWebSocket().then((ws) => assignButtons(ws));
});

function assignButtons(ws){
    $("#up").off("click");
    $("#down").off("click");
    $("#left").off("click");
    $("#right").off("click");
    $("#cross").off("click");
    
    $("#up").click(() =>  ws.send("U"));
    $("#down").click(() => ws.send("D"));
    $("#left").click(() => ws.send("L"));
    $("#right").click(() => ws.send("R"));
    $("#cross").click(() => ws.send("X"));
}

function connectWebSocket() {
  var deferred = new $.Deferred();
  
  infoMessage("Socket connecting");
  
  var ws = new WebSocket('ws://192.168.1.210/control.ws');
  ws.onopen = () => {
    deferred.resolve(ws);
    successMessage("Socket opened").then(() => clearMessage());
  };

  ws.onclose = (e) => {
    warningMessage("Socket closed");
    setTimeout(() => connectWebSocket(), 5000);
  };

  ws.onerror = (err) => {
    errorMessage("Socket error");
    ws.close();
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