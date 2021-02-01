

#include <Arduino.h>
#include <ArduinoJson.hpp>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Update.h>
#include <WiFi.h>

#include <esp32s2/rom/rtc.h>
#include <esp_log.h>
#include <esp_ota_ops.h>
#include <esp_task_wdt.h>
#include <esp_wifi.h>
#include <soc/rtc_wdt.h>

#include "Configuration.hpp"
#include "Control.hpp"
#include "Files.hpp"
#include "Sensors.hpp"
#include "WebInterface.hpp"

namespace WebInterface
{
    namespace
    {
        auto webServer{std::unique_ptr<AsyncWebServer>{}};
        auto controlWs{AsyncWebSocket{"/control.ws"}};
        auto sensorsWs{AsyncWebSocket{"/sensors.ws"}};
        auto calibrationWs{AsyncWebSocket{"/calibration.ws"}};

        auto modeCheckTimer{0UL};
        auto accessPointTimer{0UL};
        auto sensorsSendTimer{0UL};
        auto wsCleanupTimer{0UL};
        auto calibrationStartTimer{0UL};

        auto calibrateGyro{false};
        auto calibrateAccel{false};
        auto calibrateMag{false};

        namespace Get
        {
            auto handleJqueryJsGz(AsyncWebServerRequest *request) -> void
            {
                log_d("GET /jquery.min.js.gz");

                auto response{request->beginResponse_P(200, "application/javascript", jquery_min_js_gz_start, static_cast<size_t>(jquery_min_js_gz_end - jquery_min_js_gz_start))};
                response->addHeader("Content-Encoding", "gzip");
                request->send(response);
            }

            auto handleChartMinJsGz(AsyncWebServerRequest *request) -> void
            {
                log_d("GET /chart.min.js.gz");

                auto response{request->beginResponse_P(200, "application/javascript", chart_min_js_gz_start, static_cast<size_t>(chart_min_js_gz_end - chart_min_js_gz_start))};
                response->addHeader("Content-Encoding", "gzip");
                request->send(response);
            }

            auto handleConfigurationHtml(AsyncWebServerRequest *request) -> void
            {
                log_d("GET /configuration.html");

                request->send_P(200, "text/html", configuration_html_start, static_cast<size_t>(configuration_html_end - configuration_html_start));
            }

            auto handleConfigurationJs(AsyncWebServerRequest *request) -> void
            {
                log_d("GET /configuration.js");

                request->send_P(200, "application/javascript", configuration_js_start, static_cast<size_t>(configuration_js_end - configuration_js_start));
            }

            auto handleControlHtml(AsyncWebServerRequest *request) -> void
            {
                log_d("GET /control.html");

                request->send_P(200, "text/html", control_html_start, static_cast<size_t>(control_html_end - control_html_start));
            }

            auto handleControlJs(AsyncWebServerRequest *request) -> void
            {
                log_d("GET /control.js");

                request->send_P(200, "application/javascript", control_js_start, static_cast<size_t>(control_js_end - control_js_start));
            }

            auto handleSensorsHtml(AsyncWebServerRequest *request) -> void
            {
                log_d("GET /sensors.html");

                request->send_P(200, "text/html", sensors_html_start, static_cast<size_t>(sensors_html_end - sensors_html_start));
            }

            auto handleSensorsJs(AsyncWebServerRequest *request) -> void
            {
                log_d("GET /sensors.js");

                request->send_P(200, "application/javascript", sensors_js_start, static_cast<size_t>(sensors_js_end - sensors_js_start));
            }

            auto handleMeasureHtml(AsyncWebServerRequest *request) -> void
            {
                log_d("GET /measure.html");

                request->send_P(200, "text/html", measure_html_start, static_cast<size_t>(measure_html_end - measure_html_start));
            }

            auto handleMeasureJs(AsyncWebServerRequest *request) -> void
            {
                log_d("GET /measure.js");

                request->send_P(200, "application/javascript", measure_js_start, static_cast<size_t>(measure_js_end - measure_js_start));
            }

            auto handleStyleCss(AsyncWebServerRequest *request) -> void
            {
                log_d("GET /style.css");

                request->send_P(200, "text/css", style_css_start, static_cast<size_t>(style_css_end - style_css_start));
            }

            auto handleConfigurationJson(AsyncWebServerRequest *request) -> void
            {
                log_d("GET /configuration.json");

                auto response{new AsyncJsonResponse{false, 2048}};
                auto &responseJson{response->getRoot()};

                cfg.serialize(responseJson);

                response->setLength();
                request->send(response);
            }

            auto handleCalibrateJson(AsyncWebServerRequest *request) -> void
            {
                if (request->url().endsWith("gyroscope.json"))
                {
                    log_d("GET /calibrate/gyroscope.json");
                    if (Sensors::calibrateGyroscope())
                    {
                        request->send(204);
                    }
                    else
                    {
                        request->send(500);
                    }
                }
                else if (request->url().endsWith("accelerometer.json"))
                {
                    log_d("GET /calibrate/accelerometer.json");
                    if (Sensors::calibrateAccelerometer())
                    {
                        request->send(204);
                    }
                    else
                    {
                        request->send(500);
                    }
                }
                else if (request->url().endsWith("magnetometer.json"))
                {
                    log_d("GET /calibrate/magnetometer.json");
                    if (Sensors::calibrateMagnetometer())
                    {
                        request->send(204);
                    }
                    else
                    {
                        request->send(500);
                    }
                }
            }

            auto handleModelTflite(AsyncWebServerRequest *request) -> void
            {
                log_d("GET /model.tflite");

                request->send(SPIFFS, "/model.tflite", "application/octet-stream", true);
            }

        } // namespace Get

        namespace File
        {
            auto handleFirmwareBin(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) -> void
            {
                if (index == 0)
                {
                    log_d("POST /firmware.bin");

                    if (not filename.endsWith(".bin"))
                    {
                        request->send(400, "text/plain", "File extension must be .bin");
                        return;
                    }

                    if (request->contentLength() > 1945600)
                    {
                        request->send(400, "text/plain", "File size must be 1945600 bytes or less");
                        return;
                    }

                    if (Update.size() > 0)
                    {
                        request->send(500, "text/plain", "Already uploading");
                        return;
                    }

                    if (not Update.begin(request->contentLength()))
                    {
                        request->send(500, "text/plain", Update.errorString());
                        return;
                    }
                }

                if (Update.write(data, len) != len)
                {
                    request->send(500, "text/plain", Update.errorString());
                    return;
                }

                if (final)
                {
                    if (not Update.end(true))
                    {
                        request->send(500, "text/plain", Update.errorString());
                        return;
                    }
                    request->send(200, "text/plain", "Success, rebooting in 3 seconds");
                    delay(3000);
                    ESP.restart();
                }
            }

            auto handleModelTflite(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) -> void
            {
                static auto file{fs::File{}};

                if (index == 0)
                {
                    log_d("POST /model.tflite");

                    if (not filename.endsWith(".tflite"))
                    {
                        request->send(400, "text/plain", "File extension must be .tflite");
                        return;
                    }

                    if (request->contentLength() > 16384)
                    {
                        request->send(400, "text/plain", "File size must be 16384 bytes or less");
                        return;
                    }

                    if (file.size() > 0)
                    {
                        request->send(500, "text/plain", "Already uploading");
                        return;
                    }

                    file = SPIFFS.open("/model.tflite", FILE_WRITE);
                    if (not file)
                    {
                        request->send(500, "text/plain", "Error opening file");
                        return;
                    }
                }

                if (file.write(data, len) != len)
                {
                    request->send(500, "text/plain", "Error writing to file, probably there's no space left");

                    file.close();
                    SPIFFS.remove("/model.tflite");

                    return;
                }

                if (final)
                {
                    file.close();

                    request->send(200, "text/plain", "Success, rebooting in 3 seconds");
                    delay(3000);
                    ESP.restart();
                }
            }
        } // namespace File

        namespace Post
        {
            auto handleConfigurationJson(AsyncWebServerRequest *request, JsonVariant &requestJson) -> void
            {
                log_d("POST /configuration.json");

                auto response{new AsyncJsonResponse{}};
                auto &responseJson{response->getRoot()};

                auto newCfg{cfg};
                newCfg.deserialize(requestJson);
                Configuration::save(newCfg);

                responseJson.set("Configuration saved, restarting in 3 seconds");
                response->setLength();
                request->send(response);

                delay(3000);
                esp_restart();
            }

            auto handleFile(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) -> void
            {
                if (request->url() == "/firmware.bin")
                {
                    File::handleFirmwareBin(request, filename, index, data, len, final);
                }
                else if (request->url() == "/model.tflite")
                {
                    File::handleModelTflite(request, filename, index, data, len, final);
                }
                else
                {
                    request->send(404, "not found");
                }
            }
        } // namespace Post

        namespace WebSocket
        {
            auto handleControlWs(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) -> void
            {
                if (type == WS_EVT_CONNECT)
                {
                    log_d("WS %s (%u) connect", server->url(), client->id());
                    client->ping();
                }
                else if (type == WS_EVT_DISCONNECT)
                {
                    log_d("WS %s (%u) disconnect", server->url(), client->id());
                }
                else if (type == WS_EVT_ERROR)
                {
                    log_e("WS %s (%u) error (%u): %s", server->url(), client->id(), *reinterpret_cast<const uint16_t *>(arg), reinterpret_cast<const char *>(data));
                }
                else if (type == WS_EVT_PONG)
                {
                    log_d("WS %s (%u) pong", server->url(), client->id());
                }
                else if (type == WS_EVT_DATA)
                {
                    auto info{reinterpret_cast<const AwsFrameInfo *>(arg)};
                    if (info->opcode == WS_TEXT and info->final and info->index == 0)
                    {
                        const auto text{reinterpret_cast<const char *>(data)};
                        if (info->len == 1 and strncmp(text, "U", 1) == 0)
                        {
                            Control::action(Manual::MOVE_FORWARD);
                        }
                        else if (info->len == 1 and strncmp(text, "D", 1) == 0)
                        {
                            Control::action(Manual::MOVE_BACKWARD);
                        }
                        else if (info->len == 1 and strncmp(text, "L", 1) == 0)
                        {
                            Control::action(Manual::ROTATE_LEFT);
                        }
                        else if (info->len == 1 and strncmp(text, "R", 1) == 0)
                        {
                            Control::action(Manual::ROTATE_RIGHT);
                        }
                        else if (info->len == 1 and strncmp(text, "X", 1) == 0)
                        {
                            Control::action(Manual::STOP);
                        }
                        else if (info->len == 5 and strncmp(text, "manual", 5) == 0)
                        {
                            Control::mode(Mode::MANUAL);
                        }
                        else if (info->len == 4 and strncmp(text, "auto", 4) == 0)
                        {
                            Control::mode(Mode::AUTO);
                        }
                        else if (info->len == 4 and strncmp(text, "stop", 4) == 0)
                        {
                            Control::action(Auto::STOP);
                        }
                        else if (info->len == 5 and strncmp(text, "start", 5) == 0)
                        {
                            Control::action(Auto::START);
                        }
                    }
                }
            }

            auto handleSensorsWs(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) -> void
            {
                if (type == WS_EVT_CONNECT)
                {
                    log_d("WS %s (%u) connect", server->url(), client->id());
                    client->ping();
                }
                else if (type == WS_EVT_DISCONNECT)
                {
                    log_d("WS %s (%u) disconnect", server->url(), client->id());
                }
                else if (type == WS_EVT_ERROR)
                {
                    log_e("WS %s (%u) error (%u): %s", server->url(), client->id(), *reinterpret_cast<const uint16_t *>(arg), reinterpret_cast<const char *>(data));
                }
                else if (type == WS_EVT_PONG)
                {
                    log_d("WS %s (%u) pong", server->url(), client->id());
                }
                else if (type == WS_EVT_DATA)
                {
                    // NOTHING
                }
            }

            auto handleCalibrationWs(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) -> void
            {
                if (type == WS_EVT_CONNECT)
                {
                    log_d("WS %s (%u) connect", server->url(), client->id());
                    client->ping();
                }
                else if (type == WS_EVT_DISCONNECT)
                {
                    log_d("WS %s (%u) disconnect", server->url(), client->id());
                }
                else if (type == WS_EVT_ERROR)
                {
                    log_e("WS %s (%u) error (%u): %s", server->url(), client->id(), *reinterpret_cast<const uint16_t *>(arg), reinterpret_cast<const char *>(data));
                }
                else if (type == WS_EVT_PONG)
                {
                    log_d("WS %s (%u) pong", server->url(), client->id());
                }
                else if (type == WS_EVT_DATA)
                {
                    auto info{reinterpret_cast<const AwsFrameInfo *>(arg)};
                    if (info->opcode == WS_TEXT and info->final and info->index == 0)
                    {
                        const auto text{reinterpret_cast<const char *>(data)};
                        if (info->len == 4 and strncmp(text, "gyro", 4) == 0)
                        {
                            if (not calibrateGyro and not calibrateAccel and not calibrateMag)
                            {
                                calibrateGyro = true;
                                calibrationStartTimer = 0;
                                calibrationWs.textAll("starting");
                            }
                            else
                            {
                                client->text("ongoing");
                            }
                        }
                        else if (info->len == 5 and strncmp(text, "accel", 5) == 0)
                        {
                            if (not calibrateGyro and not calibrateAccel and not calibrateMag)
                            {
                                calibrateAccel = true;
                                calibrationStartTimer = 0;
                                calibrationWs.textAll("starting");
                            }
                            else
                            {
                                client->text("ongoing");
                            }
                        }
                        else if (info->len == 3 and strncmp(text, "mag", 3) == 0)
                        {
                            if (not calibrateGyro and not calibrateAccel and not calibrateMag)
                            {
                                calibrateMag = true;
                                calibrationStartTimer = 0;
                                calibrationWs.textAll("starting");
                            }
                            else
                            {
                                client->text("ongoing");
                            }
                        }
                    }
                }
            }

        } // namespace WebSocket

        auto getMac() -> void
        {
            WiFi.mode(WIFI_MODE_APSTA);
            WiFi.macAddress(cfg.station.mac.data());
            WiFi.softAPmacAddress(cfg.accessPoint.mac.data());
            WiFi.mode(WIFI_MODE_NULL);
        }

        auto configureServer() -> void
        {
            webServer.release();

            if (WiFi.getMode() == WIFI_MODE_STA)
            {
                webServer.reset(new AsyncWebServer{cfg.station.port});
            }
            else if (WiFi.getMode() == WIFI_MODE_AP)
            {
                webServer.reset(new AsyncWebServer{cfg.accessPoint.port});
            }

            if (webServer)
            {
                webServer->on("/jquery.min.js.gz", HTTP_GET, Get::handleJqueryJsGz);
                webServer->on("/chart.min.js.gz", HTTP_GET, Get::handleChartMinJsGz);
                webServer->on("/configuration.html", HTTP_GET, Get::handleConfigurationHtml);
                webServer->on("/configuration.js", HTTP_GET, Get::handleConfigurationJs);
                webServer->on("/control.html", HTTP_GET, Get::handleControlHtml);
                webServer->on("/control.js", HTTP_GET, Get::handleControlJs);
                webServer->on("/sensors.html", HTTP_GET, Get::handleSensorsHtml);
                webServer->on("/sensors.js", HTTP_GET, Get::handleSensorsJs);
                webServer->on("/measure.html", HTTP_GET, Get::handleMeasureHtml);
                webServer->on("/measure.js", HTTP_GET, Get::handleMeasureJs);
                webServer->on("/style.css", HTTP_GET, Get::handleStyleCss);
                webServer->on("/configuration.json", HTTP_GET, Get::handleConfigurationJson);
                webServer->on("/model.tflite", HTTP_GET, Get::handleModelTflite);

                webServer->addHandler(new AsyncCallbackJsonWebHandler{"/configuration.json", Post::handleConfigurationJson, 2048});
                webServer->onFileUpload(Post::handleFile);

                controlWs.onEvent(WebSocket::handleControlWs);
                webServer->addHandler(&controlWs);

                sensorsWs.onEvent(WebSocket::handleSensorsWs);
                webServer->addHandler(&sensorsWs);

                calibrationWs.onEvent(WebSocket::handleCalibrationWs);
                webServer->addHandler(&calibrationWs);

                DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
                DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
                DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
                DefaultHeaders::Instance().addHeader("Access-Control-Max-Age", "86400");
                webServer->onNotFound([](AsyncWebServerRequest *request) {
                    log_d("not found = %s", request->url().c_str());
                    if (request->method() == HTTP_OPTIONS)
                    {
                        request->send(200);
                    }
                    else
                    {
                        request->send(404, "not found");
                    }
                });
                webServer->begin();
            }
        }

        auto configureStation() -> bool
        {
            log_d("begin");

            log_d("enabled = %u", cfg.station.enabled);
            log_d("mac = %02X-%02X-%02X-%02X-%02X-%02X", cfg.station.mac[0], cfg.station.mac[1], cfg.station.mac[2], cfg.station.mac[3], cfg.station.mac[4], cfg.station.mac[5]);
            log_d("ip = %u.%u.%u.%u", cfg.station.ip[0], cfg.station.ip[1], cfg.station.ip[2], cfg.station.ip[3]);
            log_d("netmask = %u.%u.%u.%u", cfg.station.netmask[0], cfg.station.netmask[1], cfg.station.netmask[2], cfg.station.netmask[3]);
            log_d("gateway = %u.%u.%u.%u", cfg.station.gateway[0], cfg.station.gateway[1], cfg.station.gateway[2], cfg.station.gateway[3]);
            log_d("port = %u", cfg.station.port);
            log_d("user = %s", cfg.station.user.data());
            log_d("password = %s", cfg.station.password.data());

            if (not cfg.station.enabled)
            {
                WiFi.mode(WIFI_MODE_NULL);
                return false;
            }

            if (not WiFi.mode(WIFI_MODE_STA))
            {
                log_d("mode error");
                return false;
            }

            WiFi.persistent(false);
            WiFi.setAutoConnect(false);
            WiFi.setAutoReconnect(true);

            if (not WiFi.config(cfg.station.ip.data(), cfg.station.gateway.data(), cfg.station.netmask.data()))
            {
                log_d("config error");
                return false;
            }

            WiFi.setHostname("Auto2");

            if (not WiFi.begin(cfg.station.user.data(), cfg.station.password.data()))
            {
                log_d("init error");
                return false;
            }

            WebInterface::configureServer();

            return true;
        }

        auto configureAccessPoint() -> bool
        {
            log_d("begin");

            log_d("enabled = %u", cfg.accessPoint.enabled);
            log_d("mac = %02X-%02X-%02X-%02X-%02X-%02X", cfg.accessPoint.mac[0], cfg.accessPoint.mac[1], cfg.accessPoint.mac[2], cfg.accessPoint.mac[3], cfg.accessPoint.mac[4], cfg.accessPoint.mac[5]);
            log_d("ip = %u.%u.%u.%u", cfg.accessPoint.ip[0], cfg.accessPoint.ip[1], cfg.accessPoint.ip[2], cfg.accessPoint.ip[3]);
            log_d("netmask = %u.%u.%u.%u", cfg.accessPoint.netmask[0], cfg.accessPoint.netmask[1], cfg.accessPoint.netmask[2], cfg.accessPoint.netmask[3]);
            log_d("gateway = %u.%u.%u.%u", cfg.accessPoint.gateway[0], cfg.accessPoint.gateway[1], cfg.accessPoint.gateway[2], cfg.accessPoint.gateway[3]);
            log_d("port = %u", cfg.accessPoint.port);
            log_d("user = %s", cfg.accessPoint.user.data());
            log_d("password = %s", cfg.accessPoint.password.data());
            log_d("duration = %u", cfg.accessPoint.duration);

            if (not cfg.accessPoint.enabled or rtc_get_reset_reason(0) != POWERON_RESET)
            {
                WiFi.mode(WIFI_MODE_NULL);
                return false;
            }

            if (not WiFi.mode(WIFI_MODE_AP))
            {
                log_d("mode error");
                return false;
            }

            WiFi.persistent(false);

            if (not WiFi.softAPConfig(cfg.accessPoint.ip.data(), cfg.accessPoint.gateway.data(), cfg.accessPoint.netmask.data()))
            {
                log_d("config error");
                return false;
            }

            WiFi.setHostname("Auto2");

            if (not WiFi.softAP(cfg.accessPoint.user.data(), cfg.accessPoint.password.data()))
            {
                log_d("init error");
                return false;
            }

            WebInterface::configureServer();

            return true;
        }

        auto checkAccessPoint(uint64_t syncTimer) -> void
        {
            if (syncTimer - modeCheckTimer >= 1000UL)
            {
                modeCheckTimer = syncTimer;

                if (WiFi.getMode() == WIFI_MODE_AP)
                {
                    if (accessPointTimer == 0)
                    {
                        accessPointTimer = syncTimer;
                    }
                    if (WiFi.softAPgetStationNum() > 0)
                    {
                        accessPointTimer = syncTimer;
                    }
                    else if (syncTimer - accessPointTimer >= cfg.accessPoint.duration * 1000UL)
                    {
                        WebInterface::configureStation();
                    }
                }
            }
        }

        auto cleanupWebSockets(uint64_t syncTimer) -> void
        {
            if (syncTimer - wsCleanupTimer >= 1000UL)
            {
                wsCleanupTimer = syncTimer;

                WebInterface::controlWs.cleanupClients(1);
                WebInterface::sensorsWs.cleanupClients(1);
            }
        }

        auto sendSensors(uint64_t syncTimer) -> void
        {
            if (syncTimer - sensorsSendTimer >= 100UL)
            {
                sensorsSendTimer = syncTimer;

                if (sensorsWs.count() > 0)
                {
                    auto doc{ArduinoJson::DynamicJsonDocument{1024}};
                    auto json{doc.as<ArduinoJson::JsonVariant>()};

                    Sensors::serialize(json);

                    auto str{String{}};
                    ArduinoJson::serializeJson(doc, str);
                    WebInterface::sensorsWs.textAll(str);
                }
            }
        }

        auto doCalibration(uint64_t syncTimer) -> void
        {
            if (calibrateGyro)
            {
                if (calibrationStartTimer == 0)
                {
                    calibrationStartTimer = syncTimer;
                }
                if (syncTimer - calibrationStartTimer >= 5000UL)
                {
                    calibrationWs.textAll("calibrating");

                    if (Sensors::calibrateGyroscope())
                    {
                        calibrationWs.textAll("done");
                    }
                    else
                    {
                        calibrationWs.textAll("fail");
                    }

                    calibrationWs.closeAll();
                    calibrateGyro = false;
                    calibrationStartTimer = 0;
                }
            }
            else if (calibrateAccel)
            {
                if (calibrationStartTimer == 0)
                {
                    calibrationStartTimer = syncTimer;
                }
                if (syncTimer - calibrationStartTimer >= 5000UL)
                {
                    calibrationWs.textAll("calibrating");

                    if (Sensors::calibrateAccelerometer())
                    {
                        calibrationWs.textAll("done");
                    }
                    else
                    {
                        calibrationWs.textAll("fail");
                    }

                    calibrationWs.closeAll();
                    calibrateAccel = false;
                    calibrationStartTimer = 0;
                }
            }
            else if (calibrateMag)
            {
                if (calibrationStartTimer == 0)
                {
                    calibrationStartTimer = syncTimer;
                }
                if (syncTimer - calibrationStartTimer >= 5000UL)
                {
                    calibrationWs.textAll("calibrating");

                    if (Sensors::calibrateMagnetometer())
                    {
                        calibrationWs.textAll("done");
                    }
                    else
                    {
                        calibrationWs.textAll("fail");
                    }

                    calibrationWs.closeAll();
                    calibrateMag = false;
                    calibrationStartTimer = 0;
                }
            }
        }
    } // namespace

    auto init() -> void
    {
        log_d("begin");

        WebInterface::getMac();

        if (not WebInterface::configureAccessPoint())
        {
            WebInterface::configureStation();
        }

        log_d("end");
    }

    auto process(uint64_t syncTimer) -> void
    {
        WebInterface::checkAccessPoint(syncTimer);
        WebInterface::cleanupWebSockets(syncTimer);
        WebInterface::sendSensors(syncTimer);
        WebInterface::doCalibration(syncTimer);
    }

} // namespace WebInterface