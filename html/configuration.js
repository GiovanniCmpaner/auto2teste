$(document).ready(() => {
    handleFiles();
    handleConfiguration();

    getConfiguration().then(() => clearMessage());
});

function handleFiles() {
    $("#neural_network").submit((event) => {
        event.preventDefault();
        if ($("#neural_network")[0].checkValidity()) {
            uploadNeuralNetwork().then(() => clearMessage());
        }
    });
    
    $("#firmware").submit((event) => {
        event.preventDefault();
        if ($("#firmware")[0].checkValidity()) {
            uploadFirmware().then(() => clearMessage());
        }
    });
}

function handleConfiguration() {
    $("#access_point").submit((event) => {
        event.preventDefault();
        if ($("#access_point")[0].checkValidity()) {
            setAccessPoint().then(() => clearMessage());
        }
    });

    $("#station").submit((event) => {
        event.preventDefault();
        if ($("#station")[0].checkValidity()) {
            setStation().then(() => clearMessage());
        }
    });
}

function setAccessPoint() {
    var cfg = {
        access_point: {
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

function setStation() {
    var cfg = {
        station: {
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

function setConfiguration(cfg) {
    var deferred = new $.Deferred();

    $.ajax({
        type: "POST",
        url: "/configuration.json",
        contentType: 'application/json',
        data: JSON.stringify(cfg),
        timeout: 5000,
        beforeSend: () => {
            $("input,select").prop("disabled", true);
            infoMessage("Saving");
        }
    })
        .done((msg) => {
            successMessage(msg ?? "Done");
            deferred.resolve();
        })
        .fail((xhr, status, error) => {
            errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
            deferred.reject();
        })
        .always(() => {
            $("input,select").prop("disabled", false);
        });
        
    return deferred.promise();
}

function getConfiguration() {
    var deferred = new $.Deferred();
    
    $.ajax({
        type: "GET",
        url: "/configuration.json",
        accepts: 'application/json',
        timeout: 5000,
        beforeSend: () => {
            $("input,select").prop("disabled", true);
            infoMessage("Loading");
        }
    })
    .done((cfg) => {
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

        successMessage("Done");
        deferred.resolve();
    })
    .fail((xhr, status, error) => {
        errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
        deferred.reject();
    })
    .always(() => {
        $("input,select").prop("disabled", false);
    });
        
    return deferred.promise();
}

function uploadNeuralNetwork(){
    var deferred = new $.Deferred();

    var file = $("#neural_network_file")[0].files[0];
    if(file.size > 32768)
    {
        errorMessage("File size must be 32768 bytes or less");
        deferred.reject();
    }
    else
    {
        var formData = new FormData();
        formData.append("neural_network.json", file, file.name);

        $.ajax({
            type: "POST",
            url: "/neural_network.json",
            contentType: false,
            processData: false,
            data: formData,
            timeout: 30000,
            beforeSend: () => {
                $("input,select").prop("disabled", true);
                infoMessage("Uploading");
            }
        })
        .done((msg) => {
            successMessage(msg ?? "Done");
            deferred.resolve();
        })
        .fail((xhr, status, error) => {
            errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
            deferred.reject();
        })
        .always(() => {
            $("input,select").prop("disabled", false);
        });
    }
        
    return deferred.promise();
}

function uploadFirmware(){
    var deferred = new $.Deferred();

    var file = $("#firmware_file")[0].files[0];
    if(file.size > 1945600)
    {
        errorMessage("File size must be 1945600 bytes or less");
        deferred.reject();
    }
    else
    {
        var formData = new FormData();
        formData.append("firmware.bin", file, file.name);

        $.ajax({
            type: "POST",
            url: "/firmware.bin",
            contentType: false,
            processData: false,
            data: formData,
            timeout: 300000,
            beforeSend: () => {
                $("input,select").prop("disabled", true);
                infoMessage("Uploading");
            }
        })
        .done((msg) => {
            successMessage(msg ?? "Done");
            deferred.resolve();
        })
        .fail((xhr, status, error) => {
            errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
            deferred.reject();
        })
        .always(() => {
            $("input,select").prop("disabled", false);
        });
    }
        
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