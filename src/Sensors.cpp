#include <utility>
#include <array>
#include <vector>

#include <Arduino.h>
#include <Wire.h>
#include <cstdlib>
#include <cstdint>

#include <esp_log.h>
#include <soc/dport_reg.h>

#include <Adafruit_VL53L0X.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_APDS9960.h>
#include <Adafruit_SSD1306.h>

#include "Sensors.hpp"
#include "Peripherals.hpp"

namespace Sensors
{
    static auto objects{std::array<Adafruit_VL53L0X, 2>{}};
    static auto distances{
        std::vector<std::pair<int, float>>{
            {0, 0.0f},
            {+30, 0.0f},
            {-30, 0.0f},
            {+90, 0.0f},
            {-90, 0.0f},
            {180, 0.0f}}};

    auto init() -> void
    {
        log_d("begin");
        {
            Wire.setPins(Peripherals::Distances::SDA, Peripherals::Distances::SCL);
            Wire.setTimeOut(2000);

            for (auto n{0}; n < Sensors::objects.size(); ++n)
            {
                auto &xshut{Peripherals::Distances::XSHUT[n]};

                pinMode(xshut, OUTPUT);
                digitalWrite(xshut, LOW);
            }
            delay(10);

            for (auto n{1}; n < Sensors::objects.size(); ++n)
            {
                auto &object{Sensors::objects[n]};
                auto &xshut{Peripherals::Distances::XSHUT[n]};

                pinMode(xshut, INPUT);
                delay(10);

                if (not object.begin(0x30 + n, true, &Wire, Adafruit_VL53L0X::VL53L0X_SENSE_DEFAULT))
                {
                    log_e("failed to start %d", n);
                }
                else
                {
                    log_e("successfully started %d", n);
                    object.startRangeContinuous();
                }
            }
        }
        log_d("end");
    }

    auto process() -> void
    {
        static auto readingTimer{0UL};
        if (millis() - readingTimer > 1000UL)
        {
            readingTimer = millis();

            for (auto n{1}; n < Sensors::objects.size(); ++n)
            {
                auto &object{Sensors::objects[n]};
                auto &distance{Sensors::distances[n]};

                if (object.isRangeComplete())
                {
                    const auto result{object.readRangeResult()};
                    const auto status{object.readRangeStatus()};
                    if (status == VL53L0X_ERROR_NONE)
                    {
                        log_d("read %d: %u", n, result);
                        distance.second = result;
                    }
                    else
                    {
                        char text[128]{};
                        VL53L0X_GetPalErrorString(status, text);
                        log_e("failed to read %d: %s", n, text);
                    }
                }
            }
        }
    }

    auto values() -> std::vector<std::pair<int, float>>
    {
        return Sensors::distances;
    }
} // namespace Sensors