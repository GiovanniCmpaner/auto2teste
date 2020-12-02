$(document).ready(() => {
    createDistancesChart();
    createRotationChart();
    createAccelerationChart();
    createMagneticChart();
    connectWebSocket();
});

var wsSensors;
var distancesChart;
var rotationChart;
var accelerationChart;
var magneticChart;

function connectWebSocket() {
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
    setTimeout(() => connectWebSocket(), 10000);
  };

  wsSensors.onerror = (evt) => {
    // NOTHING
  };
  
  wsSensors.onmessage = (evt) => {
    var sensors = JSON.parse(evt.data);  
    updateSensors(sensors);
    updateDistancesChart(sensors.dist);
    updateRotationChart(sensors.rot);
    updateAccelerationChart(sensors.accel);
    updateMagneticChart(sensors.mag);
  };
  
  return deferred.promise();
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
    
    smoothValue("#distances_0", sensors.dist["0"]);
    smoothValue("#distances_1", sensors.dist["33"]);
    smoothValue("#distances_2", sensors.dist["-33"]);
    smoothValue("#distances_3", sensors.dist["90"]);
    smoothValue("#distances_4", sensors.dist["-90"]);
    smoothValue("#distances_5", sensors.dist["180"]);
    
    smoothValue("#color_r", sensors.color[0]);
    smoothValue("#color_g", sensors.color[1]);
    smoothValue("#color_b", sensors.color[2]);
    
    smoothValue("#rotation_x", sensors.rot[0]);
    smoothValue("#rotation_y", sensors.rot[1]);
    smoothValue("#rotation_z", sensors.rot[2]);
    
    smoothValue("#acceleration_x", sensors.accel[0]);
    smoothValue("#acceleration_y", sensors.accel[1]);
    smoothValue("#acceleration_z", sensors.accel[2]);
    
    smoothValue("#magnetic_x", sensors.mag[0]);
    smoothValue("#magnetic_y", sensors.mag[1]);
    smoothValue("#magnetic_z", sensors.mag[2]);
    
    smoothValue("#temperature", sensors.temp);
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
                yAxes: [{
                    ticks: {
                        suggestedMin: 0,
                        suggestedMax: 1
                    }
                }],
                xAxes: [{
                    gridLines: {
                        drawOnChartArea: false
                    },
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
                yAxes: [{
                    ticks: {
                        suggestedMin: -5,
                        suggestedMax: 5
                    }
                }],
                xAxes: [{
                    gridLines: {
                        drawOnChartArea: false
                    },
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
                yAxes: [{
                    ticks: {
                        suggestedMin: -5,
                        suggestedMax: 5
                    }
                }],
                xAxes: [{
                    gridLines: {
                        drawOnChartArea: false
                    },
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

function createMagneticChart(){
    var ctx = document.getElementById('magnetic_chart').getContext('2d');
    magneticChart = new Chart(ctx, {
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
                yAxes: [{
                    ticks: {
                        suggestedMin: -40,
                        suggestedMax: 40
                    }
                }],
                xAxes: [{
                    gridLines: {
                        drawOnChartArea: false
                    },
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

function updateMagneticChart(magnetic){
    
    magneticChart.data.labels.push(Date.now());
    if(magneticChart.data.labels.length > 100){
        magneticChart.data.labels.shift();
    }
   
    magneticChart.data.datasets[0].data.push(magnetic[0]);
    magneticChart.data.datasets[1].data.push(magnetic[1]);
    magneticChart.data.datasets[2].data.push(magnetic[2]);
   
    magneticChart.data.datasets.forEach((dataset) => {
        if(dataset.data.length > 100){
            dataset.data.shift();
        }
    });

    magneticChart.update();
}

//$("#color_text").prop("value", sensors.color);
//$("#color_value").prop("value", colorFromText(sensors.color));
//
//function colorFromText(){
//    var ctx = document.createElement("canvas").getContext("2d");
//	ctx.fillStyle = str;
//	return ctx.fillStyle;
//}

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