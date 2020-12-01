$(document).ready(() => {
    createMeasureChart();
    connectWsSensors().then(() => connectWsControl());
});

var wsSensors;
var wsControl;
var measureChart;

function connectWsSensors() {
  var deferred = new $.Deferred();
  
  infoMessage("Socket connecting");
  
  wsSensors = new WebSocket(`ws://${window.location.host}/sensors.ws`);
  wsSensors.onopen = (evt) => {
    deferred.resolve(wsSensors);
    successMessage("Socket opened").then(() => clearMessage());
  };

  wsSensors.onclose = (evt) => {
    if(evt.wasClean){
        warningMessage("Socket closed");
    }
    else {
        errorMessage("Socket error");
    }
    setTimeout(() => connectWsSensors(), 10000);
  };

  wsSensors.onerror = (evt) => {
    // NOTHING
  };
  
  wsSensors.onmessage = (evt) => {
    var sensors = JSON.parse(evt.data);  
    updateMeasureChartChart(sensors.distances["0"]);
  };
  
  return deferred.promise();
}

function connectWsControl() {
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

function createMeasureChart(){
    var ctx = document.getElementById('measure_chart').getContext('2d');
    measureChart = new Chart(ctx, {
        type: 'line',
        data: {
            datasets: [
                { 
                    label: '0',
                    pointBackgroundColor: 'red',
                    borderColor: 'red'
                }
            ]
        },
        options: {
            spanGaps: true,
            scales: {
                xAxes: [{
                    ticks: {
                        autoSkip: true,
                        maxTicksLimit: 20
                    }
                }]
            },
            elements: {
                line: {
                    fill: false
                },
                point:{
                    radius: 1
                }
            }
        }
    });
}

function updateMeasureChart(distance){
    
    measureChart.data.labels.push(Date.now());
    measureChart.data.datasets[0].data.push(distance);
    measureChart.update();
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