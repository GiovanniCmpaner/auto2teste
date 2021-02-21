$(document).ready(() => {
		handleMeasure();
		
    createMeasureChart();
    connectWsSensors().then(() => connectWsControl());
});

let wsSensors = null;
let wsControl = null;
let measureChart = null;
let moveTimer = null;

let measuring = false;
let id = 0;
let start = 0;

function handleMeasure()
{
	$("#start").click((event) =>
	{
		if(measuring)
		{
			return;
		}
		
		clearDataTable();
		clearMeasureChart();
		measuring = true;
		
		wsControl.send("U");
		moveTimer = setInterval(() => wsControl.send("U"), 30);
		
		setTimeout(() => 
		{
			measuring = false;

			clearTimeout(moveTimer);
			wsControl.send("X");
		}, 2000);
	});
}

function connectWsSensors() {
  var deferred = new $.Deferred();
  
  infoMessage("Sensors socket connecting");
  
  //wsSensors = new WebSocket(`ws://${window.location.host}/sensors.ws`);
  wsSensors = new WebSocket(`ws://192.168.1.210/sensors.ws`);
  wsSensors.onopen = (evt) => {
    deferred.resolve(wsSensors);
    successMessage("Sensors socket opened").then(() => clearMessage());
  };

  wsSensors.onclose = (evt) => 
	{
    if(evt.wasClean){
        warningMessage("Sensors socket closed");
    }
    else {
        errorMessage("Sensors socket error");
    }
    setTimeout(() => connectWsSensors(), 10000);
  };

  wsSensors.onerror = (evt) => 
	{
    // NOTHING
  };
  
  wsSensors.onmessage = (evt) => 
	{
		if(measuring)
		{
			var sensors = JSON.parse(evt.data);  
			updateMeasureChart(sensors.dist["180"]);
			updateDataTable(sensors.dist["180"]);
		}
  };
  
  return deferred.promise();
}

function connectWsControl() {
  var deferred = new $.Deferred();
  
  infoMessage("Control socket connecting");
  
  //wsControl = new WebSocket(`ws://${window.location.host}/control.ws`);
  wsControl = new WebSocket(`ws://192.168.1.210/control.ws`);
  
  wsControl.onopen = (evt) => {
    deferred.resolve(wsControl);
    successMessage("Control socket opened").then(() => clearMessage());
  };

  wsControl.onclose = (evt) => {
    if(evt.wasClean){
        warningMessage("Control socket closed");
    }
    else {
        errorMessage("Control socket error");
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

function createMeasureChart()
{
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
                yAxes: [{
                    ticks: {
                        suggestedMin: 0,
                        suggestedMax: 2
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

function clearMeasureChart()
{
	measureChart.data.labels = [];
	measureChart.data.datasets[0].data = [];
	measureChart.update();
}

function updateMeasureChart(distance)
{
    measureChart.data.labels.push(Date.now());
    measureChart.data.datasets[0].data.push(distance);
    measureChart.update();
}

function clearDataTable()
{
	id = 0;
	start = Date.now();
	$("#data_table tbody tr").remove(); 
}

function updateDataTable(distance)
{
	let template = $($.parseHTML($("#data_template").html()));
	let row = template.clone();
	
	row.find("#data_id").text(id++);
	row.find("#data_time").text((Date.now() - start) / 1000.0);
	row.find("#data_distance").text(distance);
	for (let c of row.find("*")) {
			if (c.id) {
					c.id += `_${id}`;
			}
			if (c.htmlFor) {
					c.htmlFor += `_${id}`;
			}
	}
	row.appendTo($("#data_table tbody"));
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