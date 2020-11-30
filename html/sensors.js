$(document).ready(() => {
    connectWebSocket().then(() => drawChart());
});

var chart;

function connectWebSocket() {
  var deferred = new $.Deferred();
  
  infoMessage("Socket connecting");
  
  var ws = new WebSocket(`ws://${window.location.host}/sensors.ws`);
  ws.onopen = (evt) => {
    deferred.resolve(ws);
    successMessage("Socket opened").then(() => clearMessage());
  };

  ws.onclose = (evt) => {
    warningMessage("Socket closed");
    setTimeout(() => connectWebSocket(), 5000);
  };

  ws.onerror = (evt) => {
    errorMessage("Socket error");
    ws.close();
  };
  
  ws.onmessage = (evt) => {
    updateSensors(evt.data);
  };
  
  return deferred.promise();
}

function updateSensors(data){
    var json = JSON.parse(data);
    
    $("#distances_0").prop("value", json.distances["0"]);
    $("#distances_1").prop("value", json.distances["33"]);
    $("#distances_2").prop("value", json.distances["-33"]);
    $("#distances_3").prop("value", json.distances["90"]);
    $("#distances_4").prop("value", json.distances["-90"]);
    $("#distances_5").prop("value", json.distances["180"]);
    
    $("#color_r").prop("value", json.color[0]);
    $("#color_g").prop("value", json.color[1]);
    $("#color_b").prop("value", json.color[2]);
    
    $("#rotation_x").prop("value", json.rotation[0]);
    $("#rotation_y").prop("value", json.rotation[1]);
    $("#rotation_z").prop("value", json.rotation[2]);
    
    $("#acceleration_x").prop("value", json.acceleration[0]);
    $("#acceleration_y").prop("value", json.acceleration[1]);
    $("#acceleration_z").prop("value", json.acceleration[2]);
    
    $("#temperature").prop("value", json.temperature);
    
    chart.data.labels.push(Date.now());
    if(chart.data.labels.length > 100){
        chart.data.labels.shift();
    }
   
    chart.data.datasets[0].data.push(json.distances["-90"]);
    chart.data.datasets[1].data.push(json.distances["-33"]);
    chart.data.datasets[2].data.push(json.distances["0"]);
    chart.data.datasets[3].data.push(json.distances["33"]);
    chart.data.datasets[4].data.push(json.distances["90"]);
    chart.data.datasets[5].data.push(json.distances["180"]);
   
    chart.data.datasets.forEach((dataset) => {
        if(dataset.data.length > 100){
            dataset.data.shift();
        }
    });

    chart.update();
}

function drawChart(){
    var ctx = document.getElementById('chart').getContext('2d');
    chart = new Chart(ctx, {
        type: 'line',
        data: {
            datasets: [
                { 
                    label: '-90',
                    pointBackgroundColor: 'purple',
                    borderColor: 'purple'
                },
                { 
                    label: '-33',
                    pointBackgroundColor: 'red',
                    borderColor: 'red'
                },
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
                    label: '90',
                    pointBackgroundColor: 'yellow',
                    borderColor: 'yellow'
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