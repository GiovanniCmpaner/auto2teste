#include <array>
#include <cstdint>
#include <cstdlib>
#include <utility>
#include <vector>

#include <Arduino.h>
#include <Wire.h>
#include <esp_log.h>

#include "Peripherals.hpp"
#include "Sensors.hpp"

#include <Adafruit_APDS9960.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_VL53L0X.h>

namespace Sensors
{
    static auto colorSensor{Adafruit_APDS9960{}};
    static auto gyroAccelSensor{Adafruit_ICM20948{}};
    static auto distanceSensors{std::array<Adafruit_VL53L0X, 6>{}};

    static auto distanceValues{std::array<std::pair<int, float>, 6>{}};
    static auto colorValues{std::array<uint16_t, 3>{}};
    static auto rotationValues{std::array<float, 3>{}};
    static auto accelerationValues{std::array<float, 3>{}};
    static auto magneticValues{std::array<float, 3>{}};
    static auto temperatureValue{float{}};

    static auto rotationOffset{std::array<float, 3>{}};
    static auto accelerationOffset{std::array<float, 3>{}};

    static auto initColor() -> void
    {
        if (not colorSensor.begin(30, APDS9960_AGAIN_64X, APDS9960_ADDRESS, &Peripherals::Color::I2C))
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

    static auto initGyroAccelMag() -> void
    {
        if (not gyroAccelSensor.begin_I2C(Peripherals::GyroAccelMag::ADDRESS, &Peripherals::GyroAccelMag::I2C, 0))
        {
            log_e("failed to initialize gyroscope, accelerometer and magnetometer");
        }
        else
        {
            gyroAccelSensor.setAccelRange(ICM20948_ACCEL_RANGE_2_G);
            gyroAccelSensor.setGyroRange(ICM20948_GYRO_RANGE_500_DPS);
            gyroAccelSensor.setMagDataRate(AK09916_MAG_DATARATE_50_HZ);
            log_d("successfully initialized gyroscope, accelerometer and magnetometer");
        }
    }

    static auto initDistances() -> void
    {
        for (auto n{0}; n < Sensors::distanceSensors.size(); ++n)
        {
            auto &object{Sensors::distanceSensors[n]};
            auto &xshut{Peripherals::Distances::XSHUT[n]};

            digitalWrite(xshut, HIGH);
            delay(10);

            if (not object.begin(0x30 + n, false, &Peripherals::Distances::I2C, Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_ACCURACY))
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

    static auto readColor() -> void
    {
        if (Sensors::colorSensor.colorDataReady())
        {
            uint16_t r, g, b, c;
            Sensors::colorSensor.getColorData(&r, &g, &b, &c);
            Sensors::colorValues = {r, g, b};
        }
    }

    static auto readGyroAccelMag() -> void
    {
        sensors_event_t accel, gyro, mag, temp;
        if (Sensors::gyroAccelSensor.getEvent(&accel, &gyro, &temp, &mag))
        {
            Sensors::accelerationValues = {
                accel.acceleration.x - Sensors::accelerationOffset[0],
                accel.acceleration.y - Sensors::accelerationOffset[1],
                accel.acceleration.z - Sensors::accelerationOffset[2]};
            Sensors::rotationValues = {
                gyro.gyro.x - Sensors::rotationOffset[0],
                gyro.gyro.y - Sensors::rotationOffset[1],
                gyro.gyro.z - Sensors::rotationOffset[2]};
            Sensors::magneticValues = {
                mag.magnetic.x,
                mag.magnetic.y,
                mag.magnetic.z};
            Sensors::temperatureValue = temp.temperature;
        }
    }

    static auto readDistances() -> void
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
                    if (reading < 4000)
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
        }
    }

    auto init() -> void
    {
        log_d("begin");

        Sensors::initColor();
        Sensors::initGyroAccelMag();
        Sensors::initDistances();

        Sensors::resetOffset();

        Sensors::readDistances();
        Sensors::readGyroAccelMag();
        Sensors::readColor();

        log_d("end");
    }

    auto process() -> void
    {
        static auto readTimer{0UL};
        if (millis() - readTimer >= 10UL)
        {
            readTimer = millis();

            Sensors::readDistances();
            Sensors::readGyroAccelMag();
            Sensors::readColor();
        }
    }

    auto resetOffset() -> void
    {
        Sensors::accelerationOffset = {};
        Sensors::rotationOffset = {};

        Sensors::readGyroAccelMag();

        Sensors::accelerationOffset = Sensors::accelerationValues;
        Sensors::rotationOffset = Sensors::rotationValues;

        Sensors::accelerationValues = {};
        Sensors::rotationValues = {};
    }

    auto print() -> void
    {
        log_d("acceleration = %.2f, %.2f, %.2f (%s)", Sensors::accelerationValues[0], Sensors::accelerationValues[1], Sensors::accelerationValues[2], Sensors::accelerationUnit);
        log_d("rotation = %.2f, %.2f, %.2f (%s)", Sensors::rotationValues[0], Sensors::rotationValues[1], Sensors::rotationValues[2], Sensors::rotationUnit);
        log_d("temperature = %.2f (%s)", Sensors::temperatureValue, Sensors::temperatureUnit);
        log_d("color = %u, %u, %u", Sensors::colorValues[0], Sensors::colorValues[1], Sensors::colorValues[2]);
        for (auto [angle, distanceValue] : Sensors::distanceValues)
        {
            log_d("distance[ %d (%s) ] = %.3f (%s)", angle, Sensors::angleUnit, distanceValue, Sensors::distanceUnit);
        }
    }

    auto distances() -> std::array<std::pair<int, float>, 6>
    {
        return Sensors::distanceValues;
    }

    auto color() -> std::array<uint16_t, 3>
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

    auto serialize(ArduinoJson::JsonVariant &json) -> void
    {
        {
            auto distances{json["distances"]};
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
            auto rotation{json["rotation"]};
            for (auto rotationValue : Sensors::rotationValues)
            {
                rotation.add(rotationValue);
            }
        }
        {
            auto acceleration{json["acceleration"]};
            for (auto accelerationValue : Sensors::accelerationValues)
            {
                acceleration.add(accelerationValue);
            }
        }
        {
            auto magnetic{json["magnetic"]};
            for (auto magneticValue : Sensors::magneticValues)
            {
                magnetic.add(magneticValue);
            }
        }
        {
            auto temperature{json["temperature"]};
            temperature = Sensors::temperatureValue;
        }
    }

} // namespace Sensors