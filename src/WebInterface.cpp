#include <Arduino.h>
#include <ArduinoJson.hpp>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Update.h>
#include <WiFi.h>

#include <esp32s2/rom/rtc.h>
#include <esp_log.h>
#include <esp_task_wdt.h>
#include <esp_wifi.h>
#include <soc/rtc_wdt.h>

#include "Configuration.hpp"
#include "Files.hpp"
#include "Motors.hpp"
#include "Sensors.hpp"
#include "WebInterface.hpp"

namespace WebInterface
{
    static auto webServer{std::unique_ptr<AsyncWebServer>{}};
    static auto controlWs{AsyncWebSocket{"/control.ws"}};
    static auto sensorsWs{AsyncWebSocket{"/sensors.ws"}};
    static auto controlTimer{0UL};

    namespace Get
    {
        static auto handleJqueryJsGz(AsyncWebServerRequest *request) -> void
        {
            log_d("GET /jquery.min.js.gz");

            auto response{request->beginResponse_P(200, "application/javascript", jquery_min_js_gz_start, static_cast<size_t>(jquery_min_js_gz_end - jquery_min_js_gz_start))};
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
        }

        static auto handleChartMinJsGz(AsyncWebServerRequest *request) -> void
        {
            log_d("GET /chart.min.js.gz");

            auto response{request->beginResponse_P(200, "application/javascript", chart_min_js_gz_start, static_cast<size_t>(chart_min_js_gz_end - chart_min_js_gz_start))};
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
        }

        static auto handleConfigurationHtml(AsyncWebServerRequest *request) -> void
        {
            log_d("GET /configuration.html");

            request->send_P(200, "text/html", configuration_html_start, static_cast<size_t>(configuration_html_end - configuration_html_start));
        }

        static auto handleConfigurationJs(AsyncWebServerRequest *request) -> void
        {
            log_d("GET /configuration.js");

            request->send_P(200, "application/javascript", configuration_js_start, static_cast<size_t>(configuration_js_end - configuration_js_start));
        }

        static auto handleControlHtml(AsyncWebServerRequest *request) -> void
        {
            log_d("GET /control.html");

            request->send_P(200, "text/html", control_html_start, static_cast<size_t>(control_html_end - control_html_start));
        }

        static auto handleControlJs(AsyncWebServerRequest *request) -> void
        {
            log_d("GET /control.js");

            request->send_P(200, "application/javascript", control_js_start, static_cast<size_t>(control_js_end - control_js_start));
        }

        static auto handleSensorsHtml(AsyncWebServerRequest *request) -> void
        {
            log_d("GET /sensors.html");

            request->send_P(200, "text/html", sensors_html_start, static_cast<size_t>(sensors_html_end - sensors_html_start));
        }

        static auto handleSensorsJs(AsyncWebServerRequest *request) -> void
        {
            log_d("GET /sensors.js");

            request->send_P(200, "application/javascript", sensors_js_start, static_cast<size_t>(sensors_js_end - sensors_js_start));
        }

        static auto handleStyleCss(AsyncWebServerRequest *request) -> void
        {
            log_d("GET /style.css");

            request->send_P(200, "text/css", style_css_start, static_cast<size_t>(style_css_end - style_css_start));
        }

        static auto handleConfigurationJson(AsyncWebServerRequest *request) -> void
        {
            log_d("GET /configuration.json");

            auto response{new AsyncJsonResponse{false, 2048}};
            auto &responseJson{response->getRoot()};

            cfg.serialize(responseJson);

            response->setLength();
            request->send(response);
        }

    } // namespace Get

    namespace File
    {
        static auto handleFirmwareBin(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) -> void
        {
            if (index == 0)
            {
                log_d("POST /firmware.bin");

                if (not filename.endsWith(".bin"))
                {
                    request->send(400, "text/plain", "File extension must be .bin");
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

        static auto handleNetJson(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) -> void
        {
            static auto file{fs::File{}};

            if (index == 0)
            {
                log_d("POST /neural_network.json");

                if (not filename.endsWith(".json"))
                {
                    request->send(400, "text/plain", "File extension must be .json");
                    return;
                }

                file = SPIFFS.open("/neural_network.json", "w");
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
                SPIFFS.remove("/neural_network.json");

                return;
            }
            if (final)
            {
                if (file.size() <= 32768)
                {
                    file.close();

                    request->send(200, "text/plain", "Success, rebooting in 3 seconds");
                    delay(3000);
                    ESP.restart();
                }
                else
                {
                    file.close();
                    SPIFFS.remove("/neural_network.json");

                    request->send(400, "text/plain", "File size must be 32768 bytes or less");
                }
            }
        }
    } // namespace File

    namespace Post
    {
        static auto handleConfigurationJson(AsyncWebServerRequest *request, JsonVariant &requestJson) -> void
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

        static auto handleFile(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) -> void
        {
            if (request->url() == "/firmware.bin")
            {
                File::handleFirmwareBin(request, filename, index, data, len, final);
            }
            else if (request->url() == "/neural_network.json")
            {
                File::handleNetJson(request, filename, index, data, len, final);
            }
            else
            {
                request->send(404);
            }
        }
    } // namespace Post

    namespace WebSocket
    {
        static auto handleControlWs(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) -> void
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
                    if (info->len == 1)
                    {
                        if (text[0] == 'U')
                        {
                            Motors::move(Motors::Move::MOVE_FORWARD);
                        }
                        else if (text[0] == 'D')
                        {
                            Motors::move(Motors::Move::MOVE_BACKWARD);
                        }
                        else if (text[0] == 'L')
                        {
                            Motors::move(Motors::Move::ROTATE_LEFT);
                        }
                        else if (text[0] == 'R')
                        {
                            Motors::move(Motors::Move::ROTATE_RIGHT);
                        }
                        else if (text[0] == 'X')
                        {
                            Motors::move(Motors::Move::STOP);
                        }

                        controlTimer = millis();
                    }
                }
            }
        }

        static auto handleSensorsWs(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) -> void
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

    } // namespace WebSocket

    static auto configureServer() -> void
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
            webServer->on("/style.css", HTTP_GET, Get::handleStyleCss);
            webServer->on("/configuration.json", HTTP_GET, Get::handleConfigurationJson);

            webServer->addHandler(new AsyncCallbackJsonWebHandler("/configuration.json", Post::handleConfigurationJson, 2048));
            webServer->onFileUpload(Post::handleFile);

            controlWs.onEvent(WebSocket::handleControlWs);
            webServer->addHandler(&controlWs);

            sensorsWs.onEvent(WebSocket::handleSensorsWs);
            webServer->addHandler(&sensorsWs);

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

    static auto configureStation() -> bool
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

    static auto checkAccessPoint() -> void
    {
        {
            static auto checkTimer{0UL};
            if (millis() - checkTimer < 1000UL)
            {
                return;
            }
            checkTimer = millis();
        }

        if (WiFi.getMode() != WIFI_MODE_AP)
        {
            return;
        }

        {
            static auto modeTimer{millis()};
            if (WiFi.softAPgetStationNum() > 0)
            {
                modeTimer = millis();
            }
            else if (millis() - modeTimer >= cfg.accessPoint.duration * 1000UL)
            {
                WebInterface::configureStation();
            }
        }
    }

    static auto configureAccessPoint() -> bool
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

    static auto cleanupWebSockets() -> void
    {
        static auto cleanupTimer{0UL};
        if (millis() - cleanupTimer >= 1000UL)
        {
            cleanupTimer = millis();

            WebInterface::controlWs.cleanupClients(1);
            WebInterface::sensorsWs.cleanupClients(1);
        }
    }

    static auto sendSensors() -> void
    {
        static auto sendTimer{0UL};
        if (millis() - sendTimer >= 100UL)
        {
            sendTimer = millis();

            if (sensorsWs.count() > 0)
            {
                auto doc{ArduinoJson::DynamicJsonDocument{512}};
                auto json{doc.as<ArduinoJson::JsonVariant>()};

                Sensors::serialize(json);

                auto str{String{}};
                ArduinoJson::serializeJson(doc, str);
                WebInterface::sensorsWs.textAll(str);
            }
        }
    }

    static auto checkControlTimeout() -> void
    {
        if (controlTimer != 0 and millis() - controlTimer >= 50UL)
        {
            Motors::move(Motors::Move::STOP);
            controlTimer = 0;
        }
    }

    auto init() -> void
    {
        log_d("begin");

        WiFi.mode(WIFI_MODE_APSTA);
        WiFi.macAddress(cfg.station.mac.data());
        WiFi.softAPmacAddress(cfg.accessPoint.mac.data());
        WiFi.mode(WIFI_MODE_NULL);

        if (not WebInterface::configureAccessPoint())
        {
            WebInterface::configureStation();
        }

        log_d("end");
    }

    auto process() -> void
    {
        WebInterface::checkAccessPoint();
        WebInterface::cleanupWebSockets();
        WebInterface::sendSensors();
        WebInterface::checkControlTimeout();
    }
} // namespace WebInterface