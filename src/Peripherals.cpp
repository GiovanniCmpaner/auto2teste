#include <Arduino.h>
#include <Wire.h>

#include <esp_log.h>

#include "Peripherals.hpp"

namespace Peripherals
{
    auto init() -> void
    {
        log_d("begin");

        pinMode(Peripherals::Motors::STBY, OUTPUT);
        pinMode(Peripherals::Motors::PWM, OUTPUT);
        pinMode(Peripherals::Motors::AIN1, OUTPUT);
        pinMode(Peripherals::Motors::AIN2, OUTPUT);
        pinMode(Peripherals::Motors::BIN1, OUTPUT);
        pinMode(Peripherals::Motors::BIN2, OUTPUT);
        pinMode(Peripherals::Motors::CIN1, OUTPUT);
        pinMode(Peripherals::Motors::CIN2, OUTPUT);
        pinMode(Peripherals::Motors::DIN1, OUTPUT);
        pinMode(Peripherals::Motors::DIN2, OUTPUT);

        digitalWrite(Peripherals::Motors::STBY, LOW);
        digitalWrite(Peripherals::Motors::PWM, LOW);
        digitalWrite(Peripherals::Motors::AIN1, LOW);
        digitalWrite(Peripherals::Motors::AIN2, LOW);
        digitalWrite(Peripherals::Motors::BIN1, LOW);
        digitalWrite(Peripherals::Motors::BIN2, LOW);
        digitalWrite(Peripherals::Motors::CIN1, LOW);
        digitalWrite(Peripherals::Motors::CIN2, LOW);
        digitalWrite(Peripherals::Motors::DIN1, LOW);
        digitalWrite(Peripherals::Motors::DIN2, LOW);

        for (auto xshut : Peripherals::Distances::XSHUT)
        {
            pinMode(xshut, OUTPUT);
            digitalWrite(xshut, LOW);
        }

        Peripherals::Distances::I2C.begin(Peripherals::Distances::SDA, Peripherals::Distances::SCL);
        Peripherals::Color::I2C.begin(Peripherals::Color::SDA, Peripherals::Color::SCL);
        Peripherals::GyroAccel::I2C.begin(Peripherals::GyroAccel::SDA, Peripherals::GyroAccel::SCL);

        delay(100);

        log_d("end");
    }
} // namespace Peripherals