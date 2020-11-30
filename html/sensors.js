$(document).ready(() => {
    createDistancesChart();
    createRotationChart();
    createAccelerationChart();
    connectWebSocket();
});

var webSocket;
var pingTimer;
var distancesChart;
var rotationChart;
var accelerationChart;

function connectWebSocket() {
  var deferred = new $.Deferred();
  
  infoMessage("Socket connecting");
  
  webSocket = new WebSocket(`ws://${window.location.host}/sensors.ws`);
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
    if(evt.data == "pong")
    {
        ping();
    }
    else {
        var sensors = JSON.parse(evt.data);  
        updateSensors(sensors);
        updateDistancesChart(sensors.distances);
        updateRotationChart(sensors.rotation);
        updateAccelerationChart(sensors.acceleration);
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

function smoothValue(id, val){
    var obj = $(id);
    var factor = 0.1;
    if(val == null)
    {
         obj.prop("value", "");
    }
    else {
        if(obj.prop("value") == ""){
            obj.prop("value", val.toFixed(3));
        }
        else {
            obj.prop("value", ( factor * val + ( 1 - factor ) * obj.prop("value")).toFixed(3) );
        }
    }
}

function updateSensors(sensors){
    
    smoothValue("#distances_0", sensors.distances["0"]);
    smoothValue("#distances_1", sensors.distances["33"]);
    smoothValue("#distances_2", sensors.distances["-33"]);
    smoothValue("#distances_3", sensors.distances["90"]);
    smoothValue("#distances_4", sensors.distances["-90"]);
    smoothValue("#distances_5", sensors.distances["180"]);
    
    smoothValue("#color_r", sensors.color[0]);
    smoothValue("#color_g", sensors.color[1]);
    smoothValue("#color_b", sensors.color[2]);
    
    smoothValue("#rotation_x", sensors.rotation[0]);
    smoothValue("#rotation_y", sensors.rotation[1]);
    smoothValue("#rotation_z", sensors.rotation[2]);
    
    smoothValue("#acceleration_x", sensors.acceleration[0]);
    smoothValue("#acceleration_y", sensors.acceleration[1]);
    smoothValue("#acceleration_z", sensors.acceleration[2]);
    
    smoothValue("#temperature", sensors.temperature);
}

function createDistancesChart(){
    var ctx = document.getElementById('distances_chart').getContext('2d');
    distancesChart = new Chart(ctx, {
        type: 'line',
        data: {
            datasets: [
                { 
                    label: '0',
                    pointBackgroundColor: 'green',
                    borderColor: 'green'
                },
                { 
                    label: '33',
                    pointBackgroundColor: 'blue',
                    borderColor: 'blue'
                },
                { 
                    label: '-33',
                    pointBackgroundColor: 'red',
                    borderColor: 'red'
                },
                { 
                    label: '90',
                    pointBackgroundColor: 'yellow',
                    borderColor: 'yellow'
                },
                { 
                    label: '-90',
                    pointBackgroundColor: 'purple',
                    borderColor: 'purple'
                },
                { 
                    label: '180',
                    pointBackgroundColor: 'orange',
                    borderColor: 'orange'
                }
            ]
        },
        options: {
            spanGaps: true,
            tooltips: {
                enabled: false
            },
            hover: {
                mode: null
            },
            scales: {
                xAxes: [{
                    ticks: {
                        display: false
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

function updateDistancesChart(distances){
    
    distancesChart.data.labels.push(Date.now());
    if(distancesChart.data.labels.length > 100){
        distancesChart.data.labels.shift();
    }
   
    distancesChart.data.datasets[0].data.push(distances["0"]);
    distancesChart.data.datasets[1].data.push(distances["33"]);
    distancesChart.data.datasets[2].data.push(distances["-33"]);
    distancesChart.data.datasets[3].data.push(distances["90"]);
    distancesChart.data.datasets[4].data.push(distances["-90"]);
    distancesChart.data.datasets[5].data.push(distances["180"]);
   
    distancesChart.data.datasets.forEach((dataset) => {
        if(dataset.data.length > 100){
            dataset.data.shift();
        }
    });

    distancesChart.update();
}

function createRotationChart(){
    var ctx = document.getElementById('rotation_chart').getContext('2d');
    rotationChart = new Chart(ctx, {
        type: 'line',
        data: {
            datasets: [
                { 
                    label: 'x',
                    pointBackgroundColor: 'red',
                    borderColor: 'red'
                },
                { 
                    label: 'y',
                    pointBackgroundColor: 'green',
                    borderColor: 'green'
                },
                { 
                    label: 'z',
                    pointBackgroundColor: 'blue',
                    borderColor: 'blue'
                }
            ]
        },
        options: {
            spanGaps: true,
            tooltips: {
                enabled: false
            },
            hover: {
                mode: null
            },
            scales: {
                xAxes: [{
                    ticks: {
                        display: false
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

function updateRotationChart(rotation){
    
    rotationChart.data.labels.push(Date.now());
    if(rotationChart.data.labels.length > 100){
        rotationChart.data.labels.shift();
    }
   
    rotationChart.data.datasets[0].data.push(rotation[0]);
    rotationChart.data.datasets[1].data.push(rotation[1]);
    rotationChart.data.datasets[2].data.push(rotation[2]);
   
    rotationChart.data.datasets.forEach((dataset) => {
        if(dataset.data.length > 100){
            dataset.data.shift();
        }
    });

    rotationChart.update();
}

function createAccelerationChart(){
    var ctx = document.getElementById('acceleration_chart').getContext('2d');
    accelerationChart = new Chart(ctx, {
        type: 'line',
        data: {
            datasets: [
                { 
                    label: 'x',
                    pointBackgroundColor: 'red',
                    borderColor: 'red'
                },
                { 
                    label: 'y',
                    pointBackgroundColor: 'green',
                    borderColor: 'green'
                },
                { 
                    label: 'z',
                    pointBackgroundColor: 'blue',
                    borderColor: 'blue'
                }
            ]
        },
        options: {
            spanGaps: true,
            tooltips: {
                enabled: false
            },
            hover: {
                mode: null
            },
            scales: {
                xAxes: [{
                    ticks: {
                        display: false
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

function updateAccelerationChart(acceleration){
    
    accelerationChart.data.labels.push(Date.now());
    if(accelerationChart.data.labels.length > 100){
        accelerationChart.data.labels.shift();
    }
   
    accelerationChart.data.datasets[0].data.push(acceleration[0]);
    accelerationChart.data.datasets[1].data.push(acceleration[1]);
    accelerationChart.data.datasets[2].data.push(acceleration[2]);
   
    accelerationChart.data.datasets.forEach((dataset) => {
        if(dataset.data.length > 100){
            dataset.data.shift();
        }
    });

    accelerationChart.update();
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