#include <Arduino.h>
#include <SPIFFS.h>
#include <Wire.h>

#include <esp_log.h>

#include "Peripherals.hpp"

namespace Peripherals
{
    auto init() -> void
    {
        log_d("begin");

        pinMode(Motors::STBY, OUTPUT);
        pinMode(Motors::PWM, OUTPUT);
        pinMode(Motors::AIN1, OUTPUT);
        pinMode(Motors::AIN2, OUTPUT);
        pinMode(Motors::BIN1, OUTPUT);
        pinMode(Motors::BIN2, OUTPUT);
        pinMode(Motors::CIN1, OUTPUT);
        pinMode(Motors::CIN2, OUTPUT);
        pinMode(Motors::DIN1, OUTPUT);
        pinMode(Motors::DIN2, OUTPUT);

        digitalWrite(Motors::STBY, LOW);
        digitalWrite(Motors::PWM, LOW);
        digitalWrite(Motors::AIN1, LOW);
        digitalWrite(Motors::AIN2, LOW);
        digitalWrite(Motors::BIN1, LOW);
        digitalWrite(Motors::BIN2, LOW);
        digitalWrite(Motors::CIN1, LOW);
        digitalWrite(Motors::CIN2, LOW);
        digitalWrite(Motors::DIN1, LOW);
        digitalWrite(Motors::DIN2, LOW);

        for (auto xshut : Distances::XSHUT)
        {
            pinMode(xshut, OUTPUT);
            digitalWrite(xshut, LOW);
        }

        //pinMode(LED::CTRL, OUTPUT);
        //digitalWrite(LED::CTRL, LOW);

        Distances::I2C.begin(Distances::SDA, Distances::SCL, 400000);
        Color::I2C.begin(Color::SDA, Color::SCL, 400000);
        GyroAccelMag::I2C.begin(GyroAccelMag::SDA, GyroAccelMag::SCL, 400000);
        Display::I2C.begin(Display::SDA, Display::SCL, 400000);

        SPIFFS.begin(true);

        delay(100);

        log_d("end");
    }
} // namespace Peripherals