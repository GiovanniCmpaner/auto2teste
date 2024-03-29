$(document).ready(() =>
{
	handleFiles();
	handleConfiguration();

	getConfiguration();
});

function handleFiles()
{
	$("#model").submit((event) =>
	{
		event.preventDefault();
		
		let submitter = event.originalEvent.submitter.id;

		if (submitter == "model_upload")
		{
			if ($("#model")[0].checkValidity())
			{
				uploadModel();
			}
		}
		else if (submitter == "model_download")
		{
      window.open("model.tflite");
		}
	});

	$("#firmware").submit((event) =>
	{
		event.preventDefault();

		if ($("#firmware")[0].checkValidity())
		{
			uploadFirmware();
		}
	});

	$("#configuration").submit((event) =>
	{
		event.preventDefault();
		
		let submitter = event.originalEvent.submitter.id;

		if (submitter == "configuration_upload")
		{
			if ($("#configuration")[0].checkValidity())
			{
				uploadConfiguration();
			}
		}
		else if (submitter == "configuration_download")
		{
      window.open("configuration.json");
		}
	});
}

function handleConfiguration()
{
	$("#calibration").submit((event) =>
	{
		event.preventDefault();
		if ($("#calibration")[0].checkValidity())
		{
			setCalibration();
		}
	});

	$("#access_point").submit((event) =>
	{
		event.preventDefault();
		if ($("#access_point")[0].checkValidity())
		{
			setAccessPoint();
		}
	});

	$("#station").submit((event) =>
	{
		event.preventDefault();
		if ($("#station")[0].checkValidity())
		{
			setStation();
		}
	});
}

function setCalibration()
{
	let cfg = {
		calibration:
		{
			motor:
			{
				speed: parseFloat($("#calibration_motor_speed").prop("value"))
			},
			battery:
			{
				bias: parseFloat($("#calibration_battery_bias").prop("value")),
				factor: parseFloat($("#calibration_battery_factor").prop("value"))
			},
			gyroscope:
			{
				bias: [
					parseFloat($("#calibration_gyroscope_bias_x").prop("value")),
					parseFloat($("#calibration_gyroscope_bias_y").prop("value")),
					parseFloat($("#calibration_gyroscope_bias_z").prop("value"))
				],
				factor: [
					parseFloat($("#calibration_gyroscope_factor_x").prop("value")),
					parseFloat($("#calibration_gyroscope_factor_y").prop("value")),
					parseFloat($("#calibration_gyroscope_factor_z").prop("value"))
				]
			},
			accelerometer:
			{
				bias: [
					parseFloat($("#calibration_accelerometer_bias_x").prop("value")),
					parseFloat($("#calibration_accelerometer_bias_y").prop("value")),
					parseFloat($("#calibration_accelerometer_bias_z").prop("value"))
				],
				factor: [
					parseFloat($("#calibration_accelerometer_factor_x").prop("value")),
					parseFloat($("#calibration_accelerometer_factor_y").prop("value")),
					parseFloat($("#calibration_accelerometer_factor_z").prop("value"))
				]
			},
			magnetometer:
			{
				bias: [
					parseFloat($("#calibration_magnetometer_bias_x").prop("value")),
					parseFloat($("#calibration_magnetometer_bias_y").prop("value")),
					parseFloat($("#calibration_magnetometer_bias_z").prop("value"))
				],
				factor: [
					parseFloat($("#calibration_magnetometer_factor_x").prop("value")),
					parseFloat($("#calibration_magnetometer_factor_y").prop("value")),
					parseFloat($("#calibration_magnetometer_factor_z").prop("value"))
				]
			},
			distance:
			{
				bias: [
					parseFloat($("#calibration_distance_bias_0").prop("value")),
					parseFloat($("#calibration_distance_bias_1").prop("value")),
					parseFloat($("#calibration_distance_bias_2").prop("value")),
					parseFloat($("#calibration_distance_bias_3").prop("value")),
					parseFloat($("#calibration_distance_bias_4").prop("value")),
					parseFloat($("#calibration_distance_bias_5").prop("value"))
				],
				factor: [
					parseFloat($("#calibration_distance_factor_0").prop("value")),
					parseFloat($("#calibration_distance_factor_1").prop("value")),
					parseFloat($("#calibration_distance_factor_2").prop("value")),
					parseFloat($("#calibration_distance_factor_3").prop("value")),
					parseFloat($("#calibration_distance_factor_4").prop("value")),
					parseFloat($("#calibration_distance_factor_5").prop("value"))
				]
			},
			color:
			{
				target: parseFloat($("#calibration_color_target_c").prop("value")),
				threshold: [
					parseFloat($("#calibration_color_threshold_r").prop("value")),
					parseFloat($("#calibration_color_threshold_g").prop("value")),
					parseFloat($("#calibration_color_threshold_b").prop("value"))
				]
			}
		}
	};
	return setConfiguration(cfg);
}

function setAccessPoint()
{
	let cfg = {
		access_point:
		{
			enabled: $("#access_point_enabled").prop("checked"),
			mac: $("#access_point_mac").prop("value").split("-").map((s) => parseInt(s, 16)),
			ip: $("#access_point_ip").prop("value").split(".").map((s) => parseInt(s, 10)),
			netmask: $("#access_point_netmask").prop("value").split(".").map((s) => parseInt(s, 10)),
			gateway: $("#access_point_gateway").prop("value").split(".").map((s) => parseInt(s, 10)),
			port: parseInt($("#access_point_port").prop("value"), 10),
			user: $("#access_point_user").prop("value"),
			password: $("#access_point_password").prop("value"),
			duration: parseInt($("#access_point_duration").prop("value"), 10)
		}
	};
	return setConfiguration(cfg);
}

function setStation()
{
	let cfg = {
		station:
		{
			enabled: $("#station_enabled").prop("checked"),
			mac: $("#station_mac").prop("value").split("-").map((s) => parseInt(s, 16)),
			ip: $("#station_ip").prop("value").split(".").map((s) => parseInt(s, 10)),
			netmask: $("#station_netmask").prop("value").split(".").map((s) => parseInt(s, 10)),
			gateway: $("#station_gateway").prop("value").split(".").map((s) => parseInt(s, 10)),
			port: parseInt($("#station_port").prop("value"), 10),
			user: $("#station_user").prop("value"),
			password: $("#station_password").prop("value")
		}
	};
	return setConfiguration(cfg);
}

function setConfiguration(cfg)
{
	let deferred = new $.Deferred();

	$.ajax(
		{
			type: "POST",
			url: "/configuration.json",
			contentType: 'application/json',
			data: JSON.stringify(cfg),
			timeout: 5000,
			beforeSend: () => {
				disableInput();
				infoMessage("Saving");
			}
		})
		.done((msg) => {
			successMessage(msg ?? "Configuration saved").then(() => reload());
			//deferred.resolve();
		})
		.fail((xhr, status, error) => {
			errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
			enableInput();
			deferred.reject();
		});
		//.always(() =>
		//{
		//	enableInput();
		//});

	return deferred.promise();
}

function getConfiguration()
{
	let deferred = new $.Deferred();

	$.ajax(
		{
			type: "GET",
			url: "/configuration.json",
			accepts: 'application/json',
			timeout: 5000,
			beforeSend: () =>
			{
				disableInput();
				infoMessage("Loading");
			}
		})
		.done((cfg) =>
		{
			$("#calibration_motor_speed").prop("value", cfg.calibration.motor.speed);

			$("#calibration_battery_bias").prop("value", cfg.calibration.battery.bias);
			$("#calibration_battery_factor").prop("value", cfg.calibration.battery.factor);

			$("#calibration_gyroscope_bias_x").prop("value", cfg.calibration.gyroscope.bias[0]);
			$("#calibration_gyroscope_bias_y").prop("value", cfg.calibration.gyroscope.bias[1]);
			$("#calibration_gyroscope_bias_z").prop("value", cfg.calibration.gyroscope.bias[2]);
			$("#calibration_gyroscope_factor_x").prop("value", cfg.calibration.gyroscope.factor[0]);
			$("#calibration_gyroscope_factor_y").prop("value", cfg.calibration.gyroscope.factor[1]);
			$("#calibration_gyroscope_factor_z").prop("value", cfg.calibration.gyroscope.factor[2]);

			$("#calibration_accelerometer_bias_x").prop("value", cfg.calibration.accelerometer.bias[0]);
			$("#calibration_accelerometer_bias_y").prop("value", cfg.calibration.accelerometer.bias[1]);
			$("#calibration_accelerometer_bias_z").prop("value", cfg.calibration.accelerometer.bias[2]);
			$("#calibration_accelerometer_factor_x").prop("value", cfg.calibration.accelerometer.factor[0]);
			$("#calibration_accelerometer_factor_y").prop("value", cfg.calibration.accelerometer.factor[1]);
			$("#calibration_accelerometer_factor_z").prop("value", cfg.calibration.accelerometer.factor[2]);

			$("#calibration_magnetometer_bias_x").prop("value", cfg.calibration.magnetometer.bias[0]);
			$("#calibration_magnetometer_bias_y").prop("value", cfg.calibration.magnetometer.bias[1]);
			$("#calibration_magnetometer_bias_z").prop("value", cfg.calibration.magnetometer.bias[2]);
			$("#calibration_magnetometer_factor_x").prop("value", cfg.calibration.magnetometer.factor[0]);
			$("#calibration_magnetometer_factor_y").prop("value", cfg.calibration.magnetometer.factor[1]);
			$("#calibration_magnetometer_factor_z").prop("value", cfg.calibration.magnetometer.factor[2]);

			$("#calibration_distance_bias_0").prop("value", cfg.calibration.distance.bias[0]);
			$("#calibration_distance_bias_1").prop("value", cfg.calibration.distance.bias[1]);
			$("#calibration_distance_bias_2").prop("value", cfg.calibration.distance.bias[2]);
			$("#calibration_distance_bias_3").prop("value", cfg.calibration.distance.bias[3]);
			$("#calibration_distance_bias_4").prop("value", cfg.calibration.distance.bias[4]);
			$("#calibration_distance_bias_5").prop("value", cfg.calibration.distance.bias[5]);
			$("#calibration_distance_factor_0").prop("value", cfg.calibration.distance.factor[0]);
			$("#calibration_distance_factor_1").prop("value", cfg.calibration.distance.factor[1]);
			$("#calibration_distance_factor_2").prop("value", cfg.calibration.distance.factor[2]);
			$("#calibration_distance_factor_3").prop("value", cfg.calibration.distance.factor[3]);
			$("#calibration_distance_factor_4").prop("value", cfg.calibration.distance.factor[4]);
			$("#calibration_distance_factor_5").prop("value", cfg.calibration.distance.factor[5]);

			$("#calibration_color_target_c").prop("value", cfg.calibration.color.target);
			$("#calibration_color_threshold_r").prop("value", cfg.calibration.color.threshold[0]);
			$("#calibration_color_threshold_g").prop("value", cfg.calibration.color.threshold[1]);
			$("#calibration_color_threshold_b").prop("value", cfg.calibration.color.threshold[2]);

			$("#access_point_enabled").prop("checked", cfg.access_point.enabled);
			$("#access_point_mac").prop("value", cfg.access_point.mac.map((n) => n.toString(16).toUpperCase().padStart(2, "0")).join("-"));
			$("#access_point_ip").prop("value", cfg.access_point.ip.map((n) => n.toString(10)).join("."));
			$("#access_point_netmask").prop("value", cfg.access_point.netmask.map((n) => n.toString(10)).join("."));
			$("#access_point_gateway").prop("value", cfg.access_point.gateway.map((n) => n.toString(10)).join("."));
			$("#access_point_port").prop("value", cfg.access_point.port);
			$("#access_point_user").prop("value", cfg.access_point.user);
			$("#access_point_password").prop("value", cfg.access_point.password);
			$("#access_point_duration").prop("value", cfg.access_point.duration);

			$("#station_enabled").prop("checked", cfg.station.enabled);
			$("#station_mac").prop("value", cfg.station.mac.map((n) => n.toString(16).toUpperCase().padStart(2, "0")).join("-"));
			$("#station_ip").prop("value", cfg.station.ip.map((n) => n.toString(10)).join("."));
			$("#station_netmask").prop("value", cfg.station.netmask.map((n) => n.toString(10)).join("."));
			$("#station_gateway").prop("value", cfg.station.gateway.map((n) => n.toString(10)).join("."));
			$("#station_port").prop("value", cfg.station.port);
			$("#station_user").prop("value", cfg.station.user);
			$("#station_password").prop("value", cfg.station.password);

			successMessage("Configuration loaded").then(() => clearMessage());
			deferred.resolve();
		})
		.fail((xhr, status, error) =>
		{
			errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
			deferred.reject();
		})
		.always(() =>
		{
			enableInput();
		});

	return deferred.promise();
}

function uploadModel()
{
	let deferred = new $.Deferred();

	let file = $("#model_file")[0].files[0];
	if (file.size > 32767)
	{
		errorMessage("File size must be 32767 bytes or less");
		deferred.reject();
	}
	else
	{
		let formData = new FormData();
		formData.append("model.tflite", file, file.name);

		$.ajax(
			{
				type: "POST",
				url: "/model.tflite",
				contentType: false,
				processData: false,
				data: formData,
				timeout: 30000,
				beforeSend: () => {
					disableInput();
					infoMessage("Uploading");
				}
			})
			.done((msg) => {
				successMessage(msg ?? "Done").then(() => reload());
				//deferred.resolve();
			})
			.fail((xhr, status, error) => {
				errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
				deferred.reject();
			});
			//.always(() =>
			//{
			//	enableInput();
			//});
	}

	return deferred.promise();
}

function uploadFirmware()
{
	let deferred = new $.Deferred();

	let file = $("#firmware_file")[0].files[0];
	if (file.size > 1945600)
	{
		errorMessage("File size must be 1945600 bytes or less");
		deferred.reject();
	}
	else
	{
		let formData = new FormData();
		formData.append("firmware.bin", file, file.name);

		$.ajax(
			{
				type: "POST",
				url: "/firmware.bin",
				contentType: false,
				processData: false,
				data: formData,
				timeout: 300000,
				beforeSend: () => {
					disableInput();
					infoMessage("Uploading");
				}
			})
			.done((msg) => {
				successMessage(msg ?? "Done").then(() => reload());
				//deferred.resolve();
			})
			.fail((xhr, status, error) => {
				errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
				deferred.reject();
			});
			//.always(() =>
			//{
			//	enableInput();
			//});
	}

	return deferred.promise();
}

function uploadConfiguration()
{
	let deferred = new $.Deferred();

	let file = $("#configuration_file")[0].files[0];
	if (file.size > 4096)
	{
		errorMessage("File size must be 4096 bytes or less");
		deferred.reject();
	}
	else
	{
		let formData = new FormData();
		formData.append("configuration.json", file, file.name);

		$.ajax(
			{
				type: "POST",
				url: "/configuration.json",
				contentType: false,
				processData: false,
				data: formData,
				timeout: 30000,
				beforeSend: () => {
					disableInput();
					infoMessage("Uploading");
				}
			})
			.done((msg) => {
				successMessage(msg ?? "Done").then(() => reload());
				//deferred.resolve();
			})
			.fail((xhr, status, error) => {
				errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
				deferred.reject();
			});
			//.always(() =>
			//{
			//	enableInput();
			//});
	}

	return deferred.promise();
}

function reload()
{
	setTimeout(() =>
	{
		infoMessage("Reloading page in 15 seconds");
		setTimeout(() => location.reload(), 15000);
	}, 3000);
}

function disableInput()
{
	$("input,select,button").prop("disabled", true);
}

function enableInput()
{
	$("input,select,button").prop("disabled", false);
}

function clearMessage()
{
	if (typeof this.fadeOutHandle != "undefined")
	{
		clearTimeout(this.fadeOutHandle);
	}
	this.fadeOutHandle = setTimeout(() => 
	{
		$("#message").fadeOut(250);
	}, 2000);
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