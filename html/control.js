$(document).ready(() => 
{
    handleMode();
    handleCapture();
    handleManual();
    handleAuto();
    
    
    connectWsControl();
});


let wsControl;
let mouseTimer;

function handleMode()
{
    $("input[name=mode]").change((event) => 
    {
        if(event.target.value == "manual")
        {
            $("#capture").css("visibility", "visible");
            $("#manual").css("visibility", "visible");
            $("#auto").css("visibility", "collapse");
            
            wsControl.send("manual");
        }
        else if(event.target.value == "auto")
        {
            $("#capture").css("visibility", "collapse");
            $("#manual").css("visibility", "collapse");
            $("#auto").css("visibility", "visible");
            
            wsControl.send("auto");
        }
    });
}

function handleCapture()
{
  $("#capture_enable").change(() =>
  {
    if($("#capture_enable").prop("checked"))
    {
      $("#capture_clear").prop("disabled", true);
      $("#capture_download").prop("disabled", true);
      
      wsControl.send("capture_enable");
    }
    else 
    {
      $("#capture_clear").prop("disabled", false);
      $("#capture_download").prop("disabled", false);
      
      wsControl.send("capture_disable");
    }
  });
  
  $("#capture_clear").click(() =>
  {
    wsControl.send("capture_clear");
  });
  
  $("#capture_download").click(() =>
  {
    window.open("capture.csv");
  });
}

function handleManual()
{
    sendWhileHolding("#up", "U", "X");
    sendWhileHolding("#down", "D", "X");
    sendWhileHolding("#left", "L", "X");
    sendWhileHolding("#right", "R", "X");
}

function handleAuto()
{
    $("#start").click(() => wsControl.send("start"));
    $("#stop").click(() => wsControl.send("stop"));
}

function sendWhileHolding(id, downVal, upVal)
{
    $(id).on("mousedown touchstart", () => 
    {
        wsControl.send(downVal);
        mouseTimer = setInterval(() => wsControl.send(downVal), 30);
    })
    .on("mouseup mouseleave touchend", () => 
    {
        clearTimeout(mouseTimer);
        wsControl.send(upVal);
    });
}

function connectWsControl() 
{
  let deferred = new $.Deferred();
  
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
    setTimeout(() => connectWsControl(), 10000);
  };

  wsControl.onerror = (evt) => {
    // NOTHING
  };
  
  wsControl.onmessage = (evt) => {
    // NOTHING
  };
  
  return deferred.promise();
}

function clearMessage() 
{
    if (typeof this.fadeOutHandle != "undefined") {
        clearTimeout(this.fadeOutHandle);
    }
    this.fadeOutHandle = setTimeout(() => $("#message").fadeOut(250), 2000);
}

function infoMessage(text) 
{
    return $("#message").prop("class", "info").text(text).fadeTo(250, 1.0).promise();
}

function successMessage(text) 
{
    return $("#message").prop("class", "success").text(text).fadeTo(250, 1.0).promise();
}

function warningMessage(text) 
{
    return $("#message").prop("class", "warning").text(text).fadeTo(250, 1.0).promise();
}

function errorMessage(text) 
{
    return $("#message").prop("class", "error").text(text).fadeTo(250, 1.0).promise();
}