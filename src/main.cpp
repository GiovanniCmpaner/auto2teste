#include <Arduino.h>

#include <esp_log.h>

#include "Configuration.hpp"
#include "Peripherals.hpp"
#include "Control.hpp"
#include "WebInterface.hpp"

void setup()
{
    Serial.begin( 115200 );
    Serial.setDebugOutput( true );

    log_d( "begin" );

    Configuration::init();
    Configuration::load( &cfg );

    Control::init();
    WebInterface::init();

    log_d( "end" );
}

void loop()
{
    Control::process();
    WebInterface::process();
    delay( 1 ); // Necessário para o ESP TCP Async
}

//#include <Arduino.h>
//#include <ESPAsyncWebServer.h>
//#include <WiFi.h>
//
//void setup()
//{
//    Serial.begin( 115200 );
//    Serial.setDebugOutput( true );
//
//    const uint8_t ip[] { 192, 168, 1, 210 };
//    const uint8_t gateway[] { 192, 168, 1, 1 };
//    const uint8_t netmask[] { 255, 255, 255, 0 };
//    const char username[] { "WORKGROUP" };
//    const char password[] { "49WNN7F3CD@22" };
//
//    WiFi.mode( WIFI_MODE_STA );
//    WiFi.config( ip, gateway, netmask );
//    WiFi.begin( username, password );
//
//    const auto server{ new AsyncWebServer{80} };
//    server->on( "/index.html", HTTP_GET, []( AsyncWebServerRequest * request )
//    {
//        request->send( 200, "text/plain", "you are at index.html" );
//    } );
//    server->begin();
//}
//
//void loop() {}