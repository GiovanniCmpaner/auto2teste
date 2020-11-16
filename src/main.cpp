#include <Arduino.h>
#include <SPIFFS.h>

#include <esp_log.h>

#include "Configuration.hpp"
#include "Peripherals.hpp"
#include "WebInterface.hpp"

#undef PI
#include "tiny_dnn/tiny_dnn.h"

void setup()
{
    delay( 1000 );
    Serial.begin( 115200 );
    Serial.setDebugOutput( true );
    log_d( "begin" );

    Peripherals::init();
    Configuration::init();
    Configuration::load( &cfg );

    WebInterface::init();

#ifdef ESP32
    teste
#endif

    if( SPIFFS.exists( "/net.json" ) )
    {
        log_d( "load net.json" );

        using namespace tiny_dnn;
        network<sequential> net{};

        net.load( "/spiffs/net.json", content_type::weights_and_model, file_format::json );

        assert( net.in_data_size() == 10 );
        assert( net.out_data_size() == 4 );

        //for( auto& layer : net )
        //{
        //    auto idx{0};
        //    layer->load( {1.0, 2.0, 3.0}, idx );
        //    for( auto& weight : layer->weights() )
        //    {
        //        weight
        //    }
        //}

        //net.fast_load()
    }


    log_d( "end" );
}

void loop()
{
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