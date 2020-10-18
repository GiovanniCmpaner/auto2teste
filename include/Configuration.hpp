#pragma once

#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <array>

struct Configuration
{
    struct Station
    {
        bool enabled;
        std::array<uint8_t, 6> mac;
        std::array<uint8_t, 4> ip;
        std::array<uint8_t, 4> netmask;
        std::array<uint8_t, 4> gateway;
        uint16_t port;
        std::string user;
        std::string password;
    };

    struct AccessPoint
    {
        bool enabled;
        std::array<uint8_t, 6> mac;
        std::array<uint8_t, 4> ip;
        std::array<uint8_t, 4> netmask;
        std::array<uint8_t, 4> gateway;
        uint16_t port;
        std::string user;
        std::string password;
        uint16_t duration;
    };

    Station station;
    AccessPoint accessPoint;

    static auto init() -> void;
    static auto load( Configuration* cfg ) -> void;
    static auto save( const Configuration& cfg ) -> void;

    auto serialize( ArduinoJson::JsonVariant& json ) const -> void;
    auto deserialize( const ArduinoJson::JsonVariant& json ) -> void;
};

extern Configuration cfg;