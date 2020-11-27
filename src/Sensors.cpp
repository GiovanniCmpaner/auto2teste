#include <utility>
#include <array>
#include <vector>

#include <Arduino.h>
#include <Wire.h>
#include <cstdlib>
#include <cstdint>

#include <esp_log.h>

#include <VL53L0X.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_APDS9960.h>
#include <Adafruit_SSD1306.h>

#include "Sensors.hpp"
#include "Peripherals.hpp"

namespace Sensors
{
    static auto objects{ std::array<VL53L0X, 6>{} };
    static auto distances{ 
        std::vector<std::pair<int, float>>{
            { 0,   0.0f },
            { +30, 0.0f },
            { -30, 0.0f },
            { +90, 0.0f },
            { -90, 0.0f },
            { 180, 0.0f }
        }
    };

    auto init() -> void
    {
        log_d( "begin" );
        {
            Wire.begin( Peripherals::Distances::SDA, Peripherals::Distances::SCL );

            for (auto n{0}; n < Sensors::objects.size(); ++n)
            {
                auto& xshut{Peripherals::Distances::XSHUT[n]};

                pinMode(xshut, OUTPUT);
                digitalWrite(xshut, LOW);
            }
            delay(100);

            for (auto n{0}; n < Sensors::objects.size(); ++n)
            {
                auto& object{Sensors::objects[n]};
                auto& xshut{Peripherals::Distances::XSHUT[n]};

                pinMode(xshut, INPUT);
                delay(10);

                object.setBus(&Wire);
                if(not object.init())
                {
                    log_e("failed to start %d", n);
                }
                else 
                {
                    object.setAddress(0x30 + n);
                    object.setTimeout(500);
                    object.startContinuous();  
                }
            }
        }
        log_d( "end" );
    }

    auto process() -> void
    {
        static auto readingTimer{0UL};
        if(millis() - readingTimer > 1000UL)
        {
            readingTimer = millis();

            for (auto n{0}; n < Sensors::objects.size(); ++n)
            {
                auto& object{Sensors::objects[n]};
                auto& distance{Sensors::distances[n]};

                const auto reading{ object.readRangeContinuousMillimeters() };
                if(reading != 65535)
                {
                    log_d("read %d: %u", n, reading);
                    distance.second = reading;
                }
            }
        }
    }

    auto values() -> std::vector<std::pair<int, float>>
    {
        return Sensors::distances;
    }
}