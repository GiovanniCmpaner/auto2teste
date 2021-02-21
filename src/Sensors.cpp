#include <array>
#include <cstdint>
#include <cstdlib>
#include <utility>
#include <vector>

#include <Arduino.h>
#include <Wire.h>
#include <esp_log.h>

#include "Configuration.hpp"
#include "Peripherals.hpp"
#include "Sensors.hpp"

#include <Adafruit_APDS9960.h>
#include <Adafruit_VL53L0X.h>
#include <MPU9250.h>

auto text(Color color) -> const char *
{
    switch (color)
    {
        case Color::BLACK:
            return "black";
        case Color::RED:
            return "red";
        case Color::GREEN:
            return "green";
        case Color::BLUE:
            return "blue";
        case Color::YELLOW:
            return "yellow";
        case Color::MAGENTA:
            return "magenta";
        case Color::CYAN:
            return "cyan";
        case Color::GRAY:
            return "gray";
        case Color::WHITE:
            return "white";
        default:
            return "unknown";
    }
}

namespace Sensors
{
    namespace
    {
        auto colorSensor{Adafruit_APDS9960{}};
        auto distanceSensors{std::array<Adafruit_VL53L0X, 6>{}};
        auto gyroAccelMagSensor{MPU9250{Peripherals::GyroAccelMag::I2C, Peripherals::GyroAccelMag::ADDRESS}};

        auto distanceValues{std::array<std::pair<int, float>, 6>{}};
        auto colorValues{std::array<float, 3>{}};
        auto rotationValues{std::array<float, 3>{}};
        auto accelerationValues{std::array<float, 3>{}};
        auto magneticValues{std::array<float, 3>{}};
        auto temperatureValue{float{}};
        auto batteryValue{float{}};

        auto readTimer{0UL};

        auto initColor() -> void
        {
            log_d("initializing color");

            if (not colorSensor.begin(50, APDS9960_AGAIN_16X, APDS9960_ADDRESS, &Peripherals::Color::I2C))
            {
                log_e("failed to initialize color");
            }
            else
            {
                colorSensor.enableColor();
            }

            ledcSetup(1, 10000, 8);
            ledcAttachPin(Peripherals::LED::CTRL, 1);
            ledcWrite(1, 0);
        }

        auto initGyroAccelMag() -> void
        {
            log_d("initializing gyroscope, accelerometer and magnetometer");

            if (gyroAccelMagSensor.begin() < 0)
            {
                log_e("failed to initialize gyroscope, accelerometer and magnetometer");
            }
            else
            {
                gyroAccelMagSensor.setAccelRange(MPU9250::ACCEL_RANGE_2G);
                gyroAccelMagSensor.setGyroRange(MPU9250::GYRO_RANGE_500DPS);
                gyroAccelMagSensor.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_41HZ);
                gyroAccelMagSensor.setSrd(19); // 50 Hz
            }
        }

        auto initDistances() -> void
        {
            for (auto n{0}; n < Sensors::distanceSensors.size(); ++n)
            {
                auto &distanceSensor{Sensors::distanceSensors[n]};
                auto &xshut{Peripherals::Distances::XSHUT[n]};

                log_d("initializing distance[%d]", n);

                digitalWrite(xshut, HIGH);
                delay(10);

                if (not distanceSensor.begin(0x30 + n, false, &Peripherals::Distances::I2C, Adafruit_VL53L0X::VL53L0X_SENSE_DEFAULT))
                {
                    log_e("failed to initialize distance[%d]", n);
                }
                else
                {
                    distanceSensor.startRangeContinuous(30);
                }
            }
        }

        auto initBattery() -> void
        {
            analogReadResolution(12);
            analogSetAttenuation(adc_attenuation_t::ADC_11db);
            analogSetClockDiv(1);
        }

        auto readColor() -> void
        {
            if (Sensors::colorSensor.colorDataReady())
            {
                uint16_t r, g, b, c;
                Sensors::colorSensor.getColorData(&r, &g, &b, &c);
                Sensors::colorValues = {
                    r * cfg.calibration.color.factor[0] + cfg.calibration.color.bias[0],
                    g * cfg.calibration.color.factor[1] + cfg.calibration.color.bias[1],
                    b * cfg.calibration.color.factor[2] + cfg.calibration.color.bias[2]};

                const auto currentDuty{ledcRead(1)};
                if (c < 500 and currentDuty < 255)
                {
                    ledcWrite(1, currentDuty + 15);
                }
                else if (c > 600 and currentDuty > 0)
                {
                    ledcWrite(1, currentDuty - 15);
                }
                //            if (c < 300)
                //            {
                //                Sensors::colorValue = Color::BLACK;
                //            }
                //            else if (c > 1500)
                //            {
                //                Sensors::colorValue = Color::WHITE;
                //            }
                //            else
                //            {
                //                if (r * 1.5 > g and r * 1.5 > b)
                //                {
                //                    Sensors::colorValue = Color::RED;
                //                }
                //                else if (g * 1.5 > r and g * 1.5 > b)
                //                {
                //                    Sensors::colorValue = Color::GREEN;
                //                }
                //                else if (b * 1.5 > r and b * 1.5 > g)
                //                {
                //                    Sensors::colorValue = Color::BLUE;
                //                }
                //                else if (r * 1.5 > b and g * 1.5 > b)
                //                {
                //                    Sensors::colorValue = Color::YELLOW;
                //                }
                //                else if (r * 1.5 > g and b * 1.5 > g)
                //                {
                //                    Sensors::colorValue = Color::MAGENTA;
                //                }
                //                else if (g * 1.5 > r and b * 1.5 > r)
                //                {
                //                    Sensors::colorValue = Color::CYAN;
                //                }
                //                else
                //                {
                //                    Sensors::colorValue = Color::GRAY;
                //                }
                //            }
            }
        }

        auto readGyroAccelMag() -> void
        {
            if (gyroAccelMagSensor.readSensor() > 0)
            {
                Sensors::accelerationValues = {
                    gyroAccelMagSensor.getAccelY_mss() * cfg.calibration.accelerometer.factor[0] + cfg.calibration.accelerometer.bias[0],
                    gyroAccelMagSensor.getAccelX_mss() * cfg.calibration.accelerometer.factor[1] + cfg.calibration.accelerometer.bias[1],
                    gyroAccelMagSensor.getAccelZ_mss() * cfg.calibration.accelerometer.factor[2] + cfg.calibration.accelerometer.bias[2]};

                Sensors::rotationValues = {
                    gyroAccelMagSensor.getGyroY_rads() * cfg.calibration.gyroscope.factor[0] + cfg.calibration.gyroscope.bias[0],
                    gyroAccelMagSensor.getGyroX_rads() * cfg.calibration.gyroscope.factor[1] + cfg.calibration.gyroscope.bias[1],
                    gyroAccelMagSensor.getGyroZ_rads() * cfg.calibration.gyroscope.factor[2] + cfg.calibration.gyroscope.bias[2]};

                Sensors::magneticValues = {
                    gyroAccelMagSensor.getMagY_uT() * cfg.calibration.magnetometer.factor[0] + cfg.calibration.magnetometer.bias[0],
                    gyroAccelMagSensor.getMagX_uT() * cfg.calibration.magnetometer.factor[1] + cfg.calibration.magnetometer.bias[1],
                    gyroAccelMagSensor.getMagZ_uT() * cfg.calibration.magnetometer.factor[2] + cfg.calibration.magnetometer.bias[2]};

                Sensors::temperatureValue = gyroAccelMagSensor.getTemperature_C();
            }
        }

        auto readDistances() -> void
        {
            for (auto n{0}; n < Sensors::distanceSensors.size(); ++n)
            {
                auto &distanceSensor{Sensors::distanceSensors[n]};
                auto &distance{Sensors::distanceValues[n]};
                auto &angle{Peripherals::Distances::ANGLES[n]};

                distance.first = angle;

                if (distanceSensor.Status == VL53L0X_ERROR_NONE)
                {
                    if (distanceSensor.isRangeComplete())
                    {
                        const auto reading{distanceSensor.readRangeResult()};
                        if (reading < 2000)
                        {
                            distance.second = (reading / 1000.0f) * cfg.calibration.distance.factor[n] + cfg.calibration.distance.bias[n];
                        }
                        else
                        {
                            //distance.second = +INFINITY;
                            distance.second = 2.0f;
                        }
                    }
                }
                else
                {
                    //distance.second = NAN;
                    distance.second = 2.0f;
                }
            }
        }

        auto readBattery() -> void
        {
            //const auto reading{analogRead(Peripherals::Battery::VIN)};
            //Sensors::batteryValue = std::clamp(reading * cfg.calibration.battery.factor + cfg.calibration.battery.bias, 0.0f, 100.0f);
        }
    } // namespace

    auto init() -> void
    {
        log_d("begin");

        Sensors::initColor();
        Sensors::initGyroAccelMag();
        Sensors::initDistances();
        Sensors::initBattery();

        Sensors::readDistances();
        Sensors::readGyroAccelMag();
        Sensors::readColor();
        Sensors::readBattery();

        log_d("end");
    }

    auto process(uint64_t syncTimer) -> void
    {
        if (syncTimer - readTimer >= 30UL)
        {
            readTimer = syncTimer;

            Sensors::readDistances();
            Sensors::readGyroAccelMag();
            Sensors::readColor();
            Sensors::readBattery();
        }
    }

    auto debug() -> void
    {
        log_d("acceleration = %.2f, %.2f, %.2f (%s)", Sensors::accelerationValues[0], Sensors::accelerationValues[1], Sensors::accelerationValues[2], Sensors::accelerationUnit);
        log_d("rotation = %.2f, %.2f, %.2f (%s)", Sensors::rotationValues[0], Sensors::rotationValues[1], Sensors::rotationValues[2], Sensors::rotationUnit);
        log_d("temperature = %.2f (%s)", Sensors::temperatureValue, Sensors::temperatureUnit);
        log_d("battery = %.2f (%s)", Sensors::batteryValue, Sensors::batteryUnit);
        log_d("color = %.2f, %.2f, %.2f", Sensors::colorValues[0], Sensors::colorValues[1], Sensors::colorValues[2]);
        for (auto [angle, distanceValue] : Sensors::distanceValues)
        {
            log_d("distance[ %d (%s) ] = %.3f (%s)", angle, Sensors::angleUnit, distanceValue, Sensors::distanceUnit);
        }
    }

    auto distances() -> std::array<std::pair<int, float>, 6>
    {
        return Sensors::distanceValues;
    }

    auto colors() -> std::array<float, 3>
    {
        return Sensors::colorValues;
    }

    auto rotation() -> std::array<float, 3>
    {
        return Sensors::rotationValues;
    }

    auto acceleration() -> std::array<float, 3>
    {
        return Sensors::accelerationValues;
    }

    auto magnetic() -> std::array<float, 3>
    {
        return Sensors::magneticValues;
    }

    auto temperature() -> float
    {
        return Sensors::temperatureValue;
    }

    auto battery() -> float
    {
        return Sensors::batteryValue;
    }

    auto serialize(ArduinoJson::JsonVariant &json) -> void
    {
        {
            auto distances{json["dist"]};
            for (auto [angle, distanceValue] : Sensors::distanceValues)
            {
                distances[String{angle}] = distanceValue;
            }
        }
        {
            auto color{json["color"]};
            for (auto colorValue : Sensors::colorValues)
            {
                color.add(colorValue);
            }
        }
        {
            auto rotation{json["rot"]};
            for (auto rotationValue : Sensors::rotationValues)
            {
                rotation.add(rotationValue);
            }
        }
        {
            auto acceleration{json["accel"]};
            for (auto accelerationValue : Sensors::accelerationValues)
            {
                acceleration.add(accelerationValue);
            }
        }
        {
            auto magnetic{json["mag"]};
            for (auto magneticValue : Sensors::magneticValues)
            {
                magnetic.add(magneticValue);
            }
        }
        {
            auto temperature{json["temp"]};
            temperature = Sensors::temperatureValue;
        }
        {
            auto battery{json["bat"]};
            battery = Sensors::batteryValue;
        }
    }

} // namespace Sensors