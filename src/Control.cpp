
#include <Arduino.h>
#include <SPIFFS.h>

#include <cstdint>
#include <esp_log.h>

#undef PI
#include <tiny_dnn/tiny_dnn.h>

#include "Control.hpp"
#include "Motors.hpp"
#include "Sensors.hpp"

namespace Control
{
    static auto load() -> void
    {
        log_d( "begin" );

        if( SPIFFS.exists( "/neural_network_loading.json" ) )
        {
            log_e( "file error" );

            SPIFFS.remove( "/neural_network_loading.json" );
        }

        if( SPIFFS.exists( "/neural_network.json" ) )
        {
            log_d( "file exists" );

            SPIFFS.rename( "/neural_network.json", "/neural_network_loading.json" );

            using namespace tiny_dnn;
            auto net{ network<sequential>{} };
            net.load( "/spiffs/neural_network_loading.json", content_type::weights_and_model, file_format::json );

            assert( net.in_data_size() == 13 );
            assert( net.out_data_size() == 4 );

            SPIFFS.rename( "/neural_network_loading.json", "/neural_network.json" );
        }

        log_d( "end" );
    }

    auto init() -> void
    {
        log_d( "begin" );

        Sensors::init();
        Motors::init();
        Control::load();

        log_d( "end" );
    }

    auto process() -> void
    {
        Sensors::process();
        Motors::process();
    }
}