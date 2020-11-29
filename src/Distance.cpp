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

#include "Distance.hpp"
#include "Peripherals.hpp"

namespace Distance
{
    static auto colorSensor{Adafruit_APDS9960{}};

    static auto giroscopeSensor{Adafruit_MPU6050{}};

    static auto distanceSensors{std::array<Adafruit_VL53L0X, 6>{}};
    static auto distanceValues{
        std::vector<std::pair<int, float>>{
            {0, NAN},
            {+30, NAN},
            {-30, NAN},
            {+90, NAN},
            {-90, NAN},
            {180, NAN}}};

    auto init() -> void
    {
        log_d("begin");

        {
            Wire1.setPins(Peripherals::Color::SDA, Peripherals::Color::SCL);

            if (not giroscopeSensor.begin(MPU6050_I2CADDR_DEFAULT, &Wire1, 0))
            {
                log_e("failed to initialize giroscope");
            }
            else
            {
                giroscopeSensor.setAccelerometerRange(MPU6050_RANGE_2_G);
                giroscopeSensor.setGyroRange(MPU6050_RANGE_500_DEG);
                giroscopeSensor.setFilterBandwidth(MPU6050_BAND_260_HZ);
                log_d("successfully initialized giroscope");
            }

            if (not colorSensor.begin(30, APDS9960_AGAIN_64X, APDS9960_ADDRESS, &Wire1))
            {
                log_e("failed to initialize color");
            }
            else
            {
                colorSensor.enableColor(true);
                colorSensor.enableGesture(false);
                colorSensor.enableProximity(false);
                log_d("successfully initialized color");
            }
        }
        {
            Wire.setPins(Peripherals::Distances::SDA, Peripherals::Distances::SCL);

            for (auto n{0}; n < Distance::distanceSensors.size(); ++n)
            {
                auto &xshut{Peripherals::Distances::XSHUT[n]};

                pinMode(xshut, OUTPUT);
                digitalWrite(xshut, LOW);
            }
            delay(10);

            for (auto n{0}; n < Distance::distanceSensors.size(); ++n)
            {
                auto &object{Distance::distanceSensors[n]};
                auto &xshut{Peripherals::Distances::XSHUT[n]};

                pinMode(xshut, INPUT);
                delay(10);

                if (not object.begin(0x30 + n, false, &Wire, Adafruit_VL53L0X::VL53L0X_SENSE_DEFAULT))
                {
                    log_e("failed to initialize distance[%d]", n);
                }
                else
                {
                    object.startRangeContinuous(30);
                    log_d("successfully initialized distance[%d]", n);
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

            log_d(" -- read -- ");

            {
                sensors_event_t a, g, t;
                if (giroscopeSensor.getEvent(&a, &g, &t))
                {
                    log_d("acceleration = %.2f, %.2f, %.2f (m/s^2)", a.acceleration.x, a.acceleration.y, a.acceleration.z);
                    log_d("rotation = %.2f, %.2f, %.2f (rad/s)", g.gyro.x, g.gyro.y, g.gyro.z);
                    log_d("temperature = %.2f (degC)", t.temperature);
                }
            }

            {
                if (colorSensor.colorDataReady())
                {
                    uint16_t r, g, b, c;
                    colorSensor.getColorData(&r, &g, &b, &c);
                    log_d("color = %u, %u, %u, %u", r, g, b, c);
                }
            }

            for (auto n{0}; n < Distance::distanceSensors.size(); ++n)
            {
                auto &object{Distance::distanceSensors[n]};
                auto &distance{Distance::distanceValues[n]};

                if (object.Status == VL53L0X_ERROR_NONE)
                {
                    if (object.isRangeComplete())
                    {
                        const auto reading{object.readRangeResult()};
                        if (reading < 8191)
                        {
                            distance.second = reading / 1000.0f;
                        }
                        else
                        {
                            distance.second = +INFINITY;
                        }
                    }
                }
                else
                {
                    distance.second = NAN;
                }

                log_d("distance[%d] = %.3f (m)", n, distance.second);
            }
        }
    }

    auto values() -> std::vector<std::pair<int, float>>
    {
        return Distance::distanceValues;
    }
} // namespace Distance