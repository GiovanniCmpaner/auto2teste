$(document).ready(() =>
{
	createDistancesChart();
	createColorChart();
	createRotationChart();
	createAccelerationChart();
	createMagneticChart();
	createBatteryChart();
	connectWsSensors();
});

let wsSensors;
let distancesChart;
let rotationChart;
let accelerationChart;
let magneticChart;

function connectWsSensors()
{
	let deferred = new $.Deferred();

	infoMessage("Socket connecting");

	wsSensors = new WebSocket(`ws://${window.location.host}/sensors.ws`);
	wsSensors.onopen = (evt) =>
	{
		deferred.resolve(wsSensors);
		successMessage("Socket opened").then(() => clearMessage());
	};

	wsSensors.onclose = (evt) =>
	{
		if (evt.wasClean)
		{
			warningMessage("Socket closed");
		}
		else
		{
			errorMessage("Socket error");
		}
		setTimeout(() => connectWsSensors(), 10000);
	};

	wsSensors.onerror = (evt) =>
	{
		// NOTHING
	};

	wsSensors.onmessage = (evt) =>
	{
		let sensors = JSON.parse(evt.data);
		updateValues(sensors);
		updateCharts(sensors);
	};

	return deferred.promise();
}

function smoothValue(id, val)
{
	let obj = $(id);
	let factor = 0.05;
	if (val == null)
	{
		obj.prop("value", "");
	}
	else
	{
		if (obj.prop("value") == "")
		{
			obj.prop("value", val.toFixed(2));
		}
		else
		{
			obj.prop("value", (factor * val + (1 - factor) * obj.prop("value")).toFixed(2));
		}
	}
}

function updateCharts(sensors)
{
	updateDistancesChart(sensors.dist);
	updateColorChart(sensors.color);
	updateRotationChart(sensors.rot);
	updateAccelerationChart(sensors.accel);
	updateMagneticChart(sensors.mag);
	updateBatteryChart(sensors.bat);
}

function updateValues(sensors)
{
	$("#distances_0").prop("value", sensors.dist["33"].toFixed(3));
	$("#distances_1").prop("value", sensors.dist["90"].toFixed(3));
	$("#distances_2").prop("value", sensors.dist["0"].toFixed(3));
	$("#distances_3").prop("value", sensors.dist["-33"].toFixed(3));
	$("#distances_4").prop("value", sensors.dist["-90"].toFixed(3));
	$("#distances_5").prop("value", sensors.dist["180"].toFixed(3));

	$("#color_name").prop("value", sensors.color["name"]);
	$("#color_r").prop("value", sensors.color["r"].toFixed(0));
	$("#color_g").prop("value", sensors.color["g"].toFixed(0));
	$("#color_b").prop("value", sensors.color["b"].toFixed(0));
	$("#color_c").prop("value", sensors.color["c"].toFixed(0));

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

	smoothValue("#battery", sensors.bat);
}

function createDistancesChart()
{
	let ctx = document.getElementById('distances_chart').getContext('2d');
	distancesChart = new Chart(ctx,
	{
		type: 'line',
		data:
		{
			datasets: [
			{
				label: '33',
				pointBackgroundColor: 'green',
				borderColor: 'green'
			},
			{
				label: '90',
				pointBackgroundColor: 'blue',
				borderColor: 'blue'
			},
			{
				label: '0',
				pointBackgroundColor: 'red',
				borderColor: 'red'
			},
			{
				label: '-33',
				pointBackgroundColor: 'yellow',
				borderColor: 'yellow'
			},
			{
				label: '-90',
				pointBackgroundColor: 'magenta',
				borderColor: 'magenta'
			},
			{
				label: '180',
				pointBackgroundColor: 'cyan',
				borderColor: 'cyan'
			}]
		},
		options:
		{
			tooltips:
			{
				enabled: false
			},
			hover:
			{
				mode: null
			},
			spanGaps: true,
			scales:
			{
				yAxes: [
				{
					ticks:
					{
						suggestedMin: 0,
						suggestedMax: 1
					}
				}],
				xAxes: [
				{
					gridLines:
					{
						drawOnChartArea: false
					},
					ticks:
					{
						display: false
					}
				}]
			},
			elements:
			{
				line:
				{
					fill: false
				},
				point:
				{
					radius: 1
				}
			}
		}
	});
}

function updateDistancesChart(distances)
{
	distancesChart.data.labels.push(Date.now());
	if (distancesChart.data.labels.length > 100)
	{
		distancesChart.data.labels.shift();
	}

	distancesChart.data.datasets[0].data.push((distances["33"] <= +9999.9 ? distances["33"] : null));
	distancesChart.data.datasets[1].data.push((distances["90"] <= +9999.9 ? distances["90"] : null));
	distancesChart.data.datasets[2].data.push((distances["0"] <= +9999.9 ? distances["0"] : null));
	distancesChart.data.datasets[3].data.push((distances["-33"] <= +9999.9 ? distances["-33"] : null));
	distancesChart.data.datasets[4].data.push((distances["-90"] <= +9999.9 ? distances["-90"] : null));
	distancesChart.data.datasets[5].data.push((distances["180"] <= +9999.9 ? distances["0"] : null));

	distancesChart.data.datasets.forEach((dataset) =>
	{
		if (dataset.data.length > 100)
		{
			dataset.data.shift();
		}
	});
	distancesChart.update();
}

function createColorChart()
{
	let ctx = document.getElementById('color_chart').getContext('2d');
	colorChart = new Chart(ctx,
	{
		type: 'line',
		data:
		{
			datasets: [
			{
				label: 'r',
				backgroundColor: '#FF0000',
				borderColor: '#FF0000'
			},
			{
				label: 'g',
				backgroundColor: '#00FF00',
				borderColor: '#00FF00'
			},
			{
				label: 'b',
				backgroundColor: '#0000FF',
				borderColor: '#0000FF'
			}]
		},
		options:
		{
			tooltips:
			{
				enabled: false
			},
			hover:
			{
				mode: null
			},
			spanGaps: true,
			scales:
			{
				yAxes: [
				{
					stacked: true,
					ticks:
					{
						suggestedMin: 0,
						suggestedMax: 600
					}
				}],
				xAxes: [
				{
					gridLines:
					{
						drawOnChartArea: false
					},
					ticks:
					{
						display: false
					}
				}]
			},
			elements:
			{
				line:
				{
					fill: true
				},
				point:
				{
					radius: 0
				}
			}
		}
	});
}

function updateColorChart(color)
{

	colorChart.data.labels.push(Date.now());
	if (colorChart.data.labels.length > 100)
	{
		colorChart.data.labels.shift();
	}

	colorChart.data.datasets[0].data.push(color["r"]);
	colorChart.data.datasets[1].data.push(color["g"]);
	colorChart.data.datasets[2].data.push(color["b"]);

	colorChart.data.datasets.forEach((dataset) =>
	{
		if (dataset.data.length > 100)
		{
			dataset.data.shift();
		}
	});

	colorChart.update();
}

function createRotationChart()
{
	let ctx = document.getElementById('rotation_chart').getContext('2d');
	rotationChart = new Chart(ctx,
	{
		type: 'line',
		data:
		{
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
			}]
		},
		options:
		{
			spanGaps: true,
			tooltips:
			{
				enabled: false
			},
			hover:
			{
				mode: null
			},
			scales:
			{
				yAxes: [
				{
					ticks:
					{
						suggestedMin: -5,
						suggestedMax: 5
					}
				}],
				xAxes: [
				{
					gridLines:
					{
						drawOnChartArea: false
					},
					ticks:
					{
						display: false
					}
				}]
			},
			elements:
			{
				line:
				{
					fill: false
				},
				point:
				{
					radius: 1
				}
			}
		}
	});
}

function updateRotationChart(rotation)
{

	rotationChart.data.labels.push(Date.now());
	if (rotationChart.data.labels.length > 100)
	{
		rotationChart.data.labels.shift();
	}

	rotationChart.data.datasets[0].data.push(rotation[0]);
	rotationChart.data.datasets[1].data.push(rotation[1]);
	rotationChart.data.datasets[2].data.push(rotation[2]);

	rotationChart.data.datasets.forEach((dataset) =>
	{
		if (dataset.data.length > 100)
		{
			dataset.data.shift();
		}
	});

	rotationChart.update();
}

function createAccelerationChart()
{
	let ctx = document.getElementById('acceleration_chart').getContext('2d');
	accelerationChart = new Chart(ctx,
	{
		type: 'line',
		data:
		{
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
			}]
		},
		options:
		{
			tooltips:
			{
				enabled: false
			},
			hover:
			{
				mode: null
			},
			spanGaps: true,
			scales:
			{
				yAxes: [
				{
					ticks:
					{
						suggestedMin: -5,
						suggestedMax: 5
					}
				}],
				xAxes: [
				{
					gridLines:
					{
						drawOnChartArea: false
					},
					ticks:
					{
						display: false
					}
				}]
			},
			elements:
			{
				line:
				{
					fill: false
				},
				point:
				{
					radius: 1
				}
			}
		}
	});
}

function updateAccelerationChart(acceleration)
{

	accelerationChart.data.labels.push(Date.now());
	if (accelerationChart.data.labels.length > 100)
	{
		accelerationChart.data.labels.shift();
	}

	accelerationChart.data.datasets[0].data.push(acceleration[0]);
	accelerationChart.data.datasets[1].data.push(acceleration[1]);
	accelerationChart.data.datasets[2].data.push(acceleration[2]);

	accelerationChart.data.datasets.forEach((dataset) =>
	{
		if (dataset.data.length > 100)
		{
			dataset.data.shift();
		}
	});

	accelerationChart.update();
}

function createMagneticChart()
{
	let ctx = document.getElementById('magnetic_chart').getContext('2d');
	magneticChart = new Chart(ctx,
	{
		type: 'line',
		data:
		{
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
			}]
		},
		options:
		{
			tooltips:
			{
				enabled: false
			},
			hover:
			{
				mode: null
			},
			spanGaps: true,
			scales:
			{
				yAxes: [
				{
					ticks:
					{
						suggestedMin: -40,
						suggestedMax: 40
					}
				}],
				xAxes: [
				{
					gridLines:
					{
						drawOnChartArea: false
					},
					ticks:
					{
						display: false
					}
				}]
			},
			elements:
			{
				line:
				{
					fill: false
				},
				point:
				{
					radius: 1
				}
			}
		}
	});
}

function updateMagneticChart(magnetic)
{
	magneticChart.data.labels.push(Date.now());
	if (magneticChart.data.labels.length > 100)
	{
		magneticChart.data.labels.shift();
	}

	magneticChart.data.datasets[0].data.push(magnetic[0]);
	magneticChart.data.datasets[1].data.push(magnetic[1]);
	magneticChart.data.datasets[2].data.push(magnetic[2]);

	magneticChart.data.datasets.forEach((dataset) =>
	{
		if (dataset.data.length > 100)
		{
			dataset.data.shift();
		}
	});

	magneticChart.update();
}

function createBatteryChart()
{
	let ctx = document.getElementById('battery_chart').getContext('2d');
	batteryChart = new Chart(ctx,
	{
		type: 'line',
		data:
		{
			datasets: [
			{
				pointBackgroundColor: 'red',
				borderColor: 'red'
			}]
		},
		options:
		{
			legend:
			{
				display: false
			},
			tooltips:
			{
				enabled: false
			},
			animation:
			{
				duration: 0
			},
			hover:
			{
				mode: null
			},
			spanGaps: true,
			responsiveAnimationDuration: 0,
			scales:
			{
				yAxes: [
				{
					ticks:
					{
						suggestedMin: 0,
						suggestedMax: 100
					}
				}],
				xAxes: [
				{
					ticks:
					{
						autoSkip: true,
						maxTicksLimit: 10
					}
				}]
			},
			elements:
			{
				line:
				{
					fill: false,
					tension: 0
				},
				point:
				{
					radius: 0
				}
			},
		}
	});
}

function updateBatteryChart(battery)
{
	if (typeof updateBatteryChart.counter == 'undefined')
	{
		updateBatteryChart.counter = 0;
	}

	if (updateBatteryChart.counter == 0)
	{
		updateBatteryChart.counter = 0;

		batteryChart.data.labels.push(new Date().toLocaleString());
		batteryChart.data.datasets[0].data.push(battery);
		batteryChart.update();
	}

	updateBatteryChart.counter++;
	if (updateBatteryChart.counter == 50)
	{
		updateBatteryChart.counter = 0;
	}
}

//$("#color_text").prop("value", sensors.color);
//$("#color_value").prop("value", colorFromText(sensors.color));
//
//function colorFromText(){
//		let ctx = document.createElement("canvas").getContext("2d");
//	ctx.fillStyle = str;
//	return ctx.fillStyle;
//}

function clearMessage()
{
	if (typeof this.fadeOutHandle != "undefined")
	{
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