#include <chrono>
#include <functional>
#include <memory>

#include <Arduino.h>
#include <ArduinoJson.hpp>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <Update.h>

#include <esp_log.h>
#include <esp_task_wdt.h>
#include <soc/rtc_wdt.h>
#include <esp32s2/rom/rtc.h>
#include <esp_wifi.h>

#include "Configuration.hpp"
#include "Peripherals.hpp"
#include "WebInterface.hpp"
#include "Motors.hpp"

extern const uint8_t control_html_start[] asm( "_binary_html_control_html_start" );
extern const uint8_t control_html_end[] asm( "_binary_html_control_html_end" );

extern const uint8_t control_js_start[] asm( "_binary_html_control_js_start" );
extern const uint8_t control_js_end[] asm( "_binary_html_control_js_end" );

extern const uint8_t configuration_html_start[] asm( "_binary_html_configuration_html_start" );
extern const uint8_t configuration_html_end[] asm( "_binary_html_configuration_html_end" );

extern const uint8_t configuration_js_start[] asm( "_binary_html_configuration_js_start" );
extern const uint8_t configuration_js_end[] asm( "_binary_html_configuration_js_end" );

extern const uint8_t style_css_start[] asm( "_binary_html_style_css_start" );
extern const uint8_t style_css_end[] asm( "_binary_html_style_css_end" );

extern const uint8_t jquery_min_js_start[] asm( "_binary_html_jquery_min_js_start" );
extern const uint8_t jquery_min_js_end[] asm( "_binary_html_jquery_min_js_end" );

namespace WebInterface
{
    namespace Get
    {
        static auto handleConfigurationHtml( AsyncWebServerRequest* request ) -> void
        {
            log_d( "GET /configuration.html" );

            request->send_P( 200, "text/html", configuration_html_start, static_cast<size_t>( configuration_html_end - configuration_html_start ) );
        }

        static auto handleConfigurationJs( AsyncWebServerRequest* request ) -> void
        {
            log_d( "GET /configuration.js" );

            request->send_P( 200, "application/javascript", configuration_js_start, static_cast<size_t>( configuration_js_end - configuration_js_start ) );
        }

        static auto handleControlHtml( AsyncWebServerRequest* request ) -> void
        {
            log_d( "GET /control.html" );

            request->send_P( 200, "text/html", control_html_start, static_cast<size_t>( control_html_end - control_html_start ) );
        }

        static auto handleControlJs( AsyncWebServerRequest* request ) -> void
        {
            log_d( "GET /control.js" );

            request->send_P( 200, "application/javascript", control_js_start, static_cast<size_t>( control_js_end - control_js_start ) );
        }

        static auto handleStyleCss( AsyncWebServerRequest* request ) -> void
        {
            log_d( "GET /style.css" );

            request->send_P( 200, "text/css", style_css_start, static_cast<size_t>( style_css_end - style_css_start ) );
        }

        static auto handleJqueryJs( AsyncWebServerRequest* request ) -> void
        {
            log_d( "GET /jquery.min.js" );

            request->send_P( 200, "application/javascript", jquery_min_js_start, static_cast<size_t>( jquery_min_js_end - jquery_min_js_start ) );
        }

        static auto handleConfigurationJson( AsyncWebServerRequest* request ) -> void
        {
            log_d( "GET /configuration.json" );

            auto response{new AsyncJsonResponse{false, 2048}};
            auto& responseJson{response->getRoot()};

            cfg.serialize( responseJson );

            response->setLength();
            request->send( response );
        }
    }

    namespace Post
    {
        static auto handleConfigurationJson( AsyncWebServerRequest* request, JsonVariant& requestJson ) -> void
        {
            log_d( "POST /configuration.json" );

            auto response{new AsyncJsonResponse{}};
            auto& responseJson{response->getRoot()};

            auto newCfg{cfg};
            newCfg.deserialize( requestJson );
            Configuration::save( newCfg );

            responseJson.set( "Configuration saved, restarting in 3 seconds" );
            response->setLength();
            request->send( response );

            delay( 3000 );
            esp_restart();
        }

        static auto handleFile( AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final ) -> void
        {
            if( request->url() != "/update" )
            {
                request->send( 404 );
                return;
            }

            log_d( "POST /update" );

            if( filename != "firmware.bin" )
            {
                request->send( 400, "text/plain", "Invalid filename" );
                return;
            }
            if( index == 0 )
            {
                Serial.printf( "UploadStart: %s\n", filename.c_str() );
                if( not Update.begin( request->contentLength() ) )
                {
                    request->send( 500, "text/plain", Update.errorString() );
                    return;
                }

            }
            if ( Update.write( data, len ) != len )
            {
                request->send( 500, "text/plain", Update.errorString() );
                return;
            }
            if( final )
            {
                if( not Update.end( true ) )
                {
                    request->send( 500, "text/plain", Update.errorString() );
                    return;
                }
                request->send( 200, "text/plain", "Success, rebooting in 3 seconds" );
                delay( 3000 );
                ESP.restart();
            }
        }
    }

    static auto wsEventHandler( AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len ) -> void
    {
        if( type == WS_EVT_CONNECT )
        {
            log_d( "ws[%s][%u] connect", server->url(), client->id() );
            client->ping();
        }
        else if( type == WS_EVT_DISCONNECT )
        {
            log_d( "ws[%s][%u] disconnect: %u", server->url(), client->id() );
        }
        else if( type == WS_EVT_ERROR )
        {
            log_e( "ws[%s][%u] error(%u): %s", server->url(), client->id(), *( ( uint16_t* )arg ), ( char* )data );
        }
        else if( type == WS_EVT_PONG )
        {
            log_d( "ws[%s][%u] pong[%u]: %s", server->url(), client->id(), len, ( len ) ? ( char* )data : "" );
        }
        else if( type == WS_EVT_DATA )
        {
            auto info{ reinterpret_cast<const AwsFrameInfo*>( arg ) };
            if( info->opcode == WS_TEXT && info->final && info->index == 0 && info->len == 1 )
            {
                const auto command{ *data };

                log_d( "ws[%s][%u] command = %c", server->url(), client->id(), command );

                if( command == 'U' )
                {
                    Motors::forward();
                }
                else if( command == 'D' )
                {
                    Motors::backward();
                }
                else if( command == 'L' )
                {
                    Motors::left();
                }
                else if( command == 'R' )
                {
                    Motors::right();
                }
                else if( command == 'X' )
                {
                    Motors::stop();
                }
            }
        }
    }

    static auto configureServer() -> void
    {
        static std::unique_ptr<AsyncWebServer> server{};
        static AsyncWebSocket ws{"/control.ws"};

        server.release();

        if ( WiFi.getMode() == WIFI_MODE_STA )
        {
            server.reset( new AsyncWebServer{cfg.station.port} );
        }
        else if ( WiFi.getMode() == WIFI_MODE_AP )
        {
            server.reset( new AsyncWebServer{cfg.accessPoint.port} );
        }

        if ( server )
        {
            server->on( "/configuration.html", HTTP_GET, Get::handleConfigurationHtml );
            server->on( "/configuration.js", HTTP_GET, Get::handleConfigurationJs );
            server->on( "/control.html", HTTP_GET, Get::handleControlHtml );
            server->on( "/control.js", HTTP_GET, Get::handleControlJs );
            server->on( "/style.css", HTTP_GET, Get::handleStyleCss );
            server->on( "/jquery.min.js", HTTP_GET, Get::handleJqueryJs );
            server->on( "/configuration.json", HTTP_GET, Get::handleConfigurationJson );

            server->addHandler( new AsyncCallbackJsonWebHandler( "/configuration.json", Post::handleConfigurationJson, 2048 ) );
            server->onFileUpload( Post::handleFile );

            ws.onEvent( wsEventHandler );
            server->addHandler( &ws );

            DefaultHeaders::Instance().addHeader( "Access-Control-Allow-Origin", "*" );
            DefaultHeaders::Instance().addHeader( "Access-Control-Allow-Methods", "POST, GET, OPTIONS" );
            DefaultHeaders::Instance().addHeader( "Access-Control-Allow-Headers", "Content-Type" );
            DefaultHeaders::Instance().addHeader( "Access-Control-Max-Age", "86400" );
            server->onNotFound( []( AsyncWebServerRequest * request )
            {
                log_d( "not found = %s", request->url().c_str() );
                if ( request->method() == HTTP_OPTIONS )
                {
                    request->send( 200 );
                }
                else
                {
                    request->send( 404 );
                }
            } );
            server->begin();
        }
    }

    static auto configureStation() -> bool
    {
        log_d( "begin" );

        log_d( "enabled = %u", cfg.station.enabled );
        log_d( "mac = %02X-%02X-%02X-%02X-%02X-%02X", cfg.station.mac[0], cfg.station.mac[1], cfg.station.mac[2], cfg.station.mac[3], cfg.station.mac[4], cfg.station.mac[5] );
        log_d( "ip = %u.%u.%u.%u", cfg.station.ip[0], cfg.station.ip[1], cfg.station.ip[2], cfg.station.ip[3] );
        log_d( "netmask = %u.%u.%u.%u", cfg.station.netmask[0], cfg.station.netmask[1], cfg.station.netmask[2], cfg.station.netmask[3] );
        log_d( "gateway = %u.%u.%u.%u", cfg.station.gateway[0], cfg.station.gateway[1], cfg.station.gateway[2], cfg.station.gateway[3] );
        log_d( "port = %u", cfg.station.port );
        log_d( "user = %s", cfg.station.user.data() );
        log_d( "password = %s", cfg.station.password.data() );

        if ( not cfg.station.enabled )
        {
            WiFi.mode( WIFI_MODE_NULL );
            return false;
        }

        if ( not WiFi.mode( WIFI_MODE_STA ) )
        {
            log_d( "mode error" );
            return false;
        }

        WiFi.persistent( false );
        WiFi.setAutoConnect( false );
        WiFi.setAutoReconnect( true );

        if ( not WiFi.config( cfg.station.ip.data(), cfg.station.gateway.data(), cfg.station.netmask.data() ) )
        {
            log_d( "config error" );
            return false;
        }

        WiFi.setHostname( "Auto2" );

        if ( not WiFi.begin( cfg.station.user.data(), cfg.station.password.data() ) )
        {
            log_d( "init error" );
            return false;
        }

        configureServer();
        return true;
    }

    static auto taskAccessPoint( void* ) -> void
    {
        auto modeTimer{ std::chrono::system_clock::now() };
        while( WiFi.getMode() == WIFI_MODE_AP )
        {
            const auto now{std::chrono::system_clock::now()};
            if( WiFi.softAPgetStationNum() > 0 )
            {
                modeTimer = now;
            }
            else
            {
                if( now - modeTimer > std::chrono::seconds( cfg.accessPoint.duration ) )
                {
                    configureStation();
                }
            }
            vTaskDelay( pdMS_TO_TICKS( 1000 ) );
        }
        vTaskDelete( nullptr );
    }

    static auto configureAccessPoint() -> bool
    {
        log_d( "begin" );

        log_d( "enabled = %u", cfg.accessPoint.enabled );
        log_d( "mac = %02X-%02X-%02X-%02X-%02X-%02X", cfg.accessPoint.mac[0], cfg.accessPoint.mac[1], cfg.accessPoint.mac[2], cfg.accessPoint.mac[3], cfg.accessPoint.mac[4], cfg.accessPoint.mac[5] );
        log_d( "ip = %u.%u.%u.%u", cfg.accessPoint.ip[0], cfg.accessPoint.ip[1], cfg.accessPoint.ip[2], cfg.accessPoint.ip[3] );
        log_d( "netmask = %u.%u.%u.%u", cfg.accessPoint.netmask[0], cfg.accessPoint.netmask[1], cfg.accessPoint.netmask[2], cfg.accessPoint.netmask[3] );
        log_d( "gateway = %u.%u.%u.%u", cfg.accessPoint.gateway[0], cfg.accessPoint.gateway[1], cfg.accessPoint.gateway[2], cfg.accessPoint.gateway[3] );
        log_d( "port = %u", cfg.accessPoint.port );
        log_d( "user = %s", cfg.accessPoint.user.data() );
        log_d( "password = %s", cfg.accessPoint.password.data() );
        log_d( "duration = %u", cfg.accessPoint.duration );

        if ( not cfg.accessPoint.enabled or rtc_get_reset_reason( 0 ) == DEEPSLEEP_RESET )
        {
            WiFi.mode( WIFI_MODE_NULL );
            return false;
        }

        if ( not WiFi.mode( WIFI_MODE_AP ) )
        {
            log_d( "mode error" );
            return false;
        }

        WiFi.persistent( false );

        if ( not WiFi.softAPConfig( cfg.accessPoint.ip.data(), cfg.accessPoint.gateway.data(), cfg.accessPoint.netmask.data() ) )
        {
            log_d( "config error" );
            return false;
        }

        WiFi.setHostname( "Auto2" );

        if ( not WiFi.softAP( cfg.accessPoint.user.data(), cfg.accessPoint.password.data() ) )
        {
            log_d( "init error" );
            return false;
        }

        configureServer();

        xTaskCreate( taskAccessPoint, "taskAccessPoint", 2048, nullptr, 1, nullptr );

        return true;
    }

    auto init() -> void
    {
        log_d( "begin" );

        if( not configureAccessPoint() )
        {
            configureStation();
        }

        log_d( "end" );
    }
}