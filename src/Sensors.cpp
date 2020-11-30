#include <utility>
#include <array>
#include <map>

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
    static auto colorSensor{Adafruit_APDS9960{}};
    static auto gyroAccelSensor{Adafruit_MPU6050{}};
    static auto distanceSensors{std::array<Adafruit_VL53L0X, 6>{}};

    static auto distanceValues{
        std::map<int, float>{
            {-90, NAN},
            {-33, NAN},
            {0, NAN},
            {+33, NAN},
            {+90, NAN},
            {180, NAN}}};

    static auto colorValues{std::array<uint16_t, 3>{}};
    static auto rotationValues{std::array<float, 3>{}};
    static auto accelerationValues{std::array<float, 3>{}};
    static auto temperatureValue{float{}};

    static auto rotationReferences{std::array<float, 3>{}};
    static auto accelerationReferences{std::array<float, 3>{}};

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

    static auto initGyroAccel() -> void
    {
        if (not gyroAccelSensor.begin(MPU6050_I2CADDR_DEFAULT, &Peripherals::GyroAccel::I2C, 0))
        {
            log_e("failed to initialize gyroscope and accelerometer");
        }
        else
        {
            gyroAccelSensor.setAccelerometerRange(MPU6050_RANGE_2_G);
            gyroAccelSensor.setGyroRange(MPU6050_RANGE_500_DEG);
            gyroAccelSensor.setFilterBandwidth(MPU6050_BAND_260_HZ);
            log_d("successfully initialized gyroscope and accelerometer");
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

    static auto readGyroAccel() -> void
    {
        sensors_event_t a, g, t;
        if (Sensors::gyroAccelSensor.getEvent(&a, &g, &t))
        {
            Sensors::accelerationValues = {
                a.acceleration.x - Sensors::accelerationReferences[0],
                a.acceleration.y - Sensors::accelerationReferences[1],
                a.acceleration.z - Sensors::accelerationReferences[2]};
            Sensors::rotationValues = {
                g.gyro.x - Sensors::rotationReferences[0],
                g.gyro.y - Sensors::rotationReferences[1],
                g.gyro.z - Sensors::rotationReferences[2]};
            Sensors::temperatureValue = t.temperature;
        }
    }

    static auto readDistances() -> void
    {
        auto it{Sensors::distanceValues.begin()};
        for (auto n{0}; n < Sensors::distanceSensors.size(); ++n, ++it)
        {
            auto &distanceSensor{Sensors::distanceSensors[n]};
            auto &distanceValue{*it};

            if (distanceSensor.Status == VL53L0X_ERROR_NONE)
            {
                if (distanceSensor.isRangeComplete())
                {
                    const auto reading{distanceSensor.readRangeResult()};
                    if (reading < 8191)
                    {
                        distanceValue.second = reading / 1000.0f;
                    }
                    else
                    {
                        distanceValue.second = +INFINITY;
                    }
                }
            }
            else
            {
                distanceValue.second = NAN;
            }
        }
    }

    auto init() -> void
    {
        log_d("begin");

        Sensors::initColor();
        Sensors::initGyroAccel();
        Sensors::initDistances();

        Sensors::resetReference();

        log_d("end");
    }

    auto process() -> void
    {
        static auto readTimer{0UL};
        if (millis() - readTimer > 10UL)
        {
            readTimer = millis();

            Sensors::readDistances();
            Sensors::readGyroAccel();
            Sensors::readColor();
        }

        //static auto printTimer{0UL};
        //if (millis() - printTimer > 5000UL)
        //{
        //    printTimer = millis();
        //
        //    Sensors::print();
        //}
    }

    auto resetReference() -> void
    {
        Sensors::accelerationReferences = {};
        Sensors::rotationReferences = {};

        Sensors::readGyroAccel();

        Sensors::accelerationReferences = Sensors::accelerationValues;
        Sensors::rotationReferences = Sensors::rotationValues;

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

    auto distances() -> std::map<int, float>
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

    auto temperature() -> float
    {
        return Sensors::temperatureValue;
    }

    auto serialize(ArduinoJson::JsonVariant &json) -> void
    {
        auto distances{json["distances"]};
        for (auto [angle, distanceValue] : Sensors::distanceValues)
        {
            distances[String{angle}] = distanceValue;
        }

        auto color{json["color"]};
        for (auto colorValue : Sensors::colorValues)
        {
            color.add(colorValue);
        }

        auto rotation{json["rotation"]};
        for (auto rotationValue : Sensors::rotationValues)
        {
            rotation.add(rotationValue);
        }

        auto acceleration{json["acceleration"]};
        for (auto accelerationValue : Sensors::accelerationValues)
        {
            acceleration.add(accelerationValue);
        }

        auto temperature{json["temperature"]};
        temperature = Sensors::temperatureValue;
    }

} // namespace Sensors