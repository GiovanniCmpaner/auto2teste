#include <Arduino.h>

#include "Peripherals.hpp"
#include "Motors.hpp"

namespace Motors
{
    auto init() -> void
    {
        log_d("begin");

        ledcSetup(0, 1000, 10);
        ledcAttachPin(Peripherals::Motors::PWM, 0);
        ledcWrite(0, 1024);

        digitalWrite(Peripherals::Motors::STBY, HIGH);

        log_d("end");
    }

    auto process() -> void
    {
    }

    auto speed(float percent) -> void
    {
        ledcWrite(0, percent / 100.0 * 1023.0);
    }

    auto forward() -> void
    {
        //digitalWrite(Peripherals::Motors::STBY, HIGH);
        //digitalWrite(Peripherals::Motors::PWM, HIGH);
        digitalWrite(Peripherals::Motors::AIN1, HIGH);
        digitalWrite(Peripherals::Motors::AIN2, LOW);
        digitalWrite(Peripherals::Motors::BIN1, HIGH);
        digitalWrite(Peripherals::Motors::BIN2, LOW);
        digitalWrite(Peripherals::Motors::CIN1, HIGH);
        digitalWrite(Peripherals::Motors::CIN2, LOW);
        digitalWrite(Peripherals::Motors::DIN1, HIGH);
        digitalWrite(Peripherals::Motors::DIN2, LOW);
    }

    auto backward() -> void
    {
        //digitalWrite(Peripherals::Motors::STBY, HIGH);
        //digitalWrite(Peripherals::Motors::PWM, HIGH);
        digitalWrite(Peripherals::Motors::AIN1, LOW);
        digitalWrite(Peripherals::Motors::AIN2, HIGH);
        digitalWrite(Peripherals::Motors::BIN1, LOW);
        digitalWrite(Peripherals::Motors::BIN2, HIGH);
        digitalWrite(Peripherals::Motors::CIN1, LOW);
        digitalWrite(Peripherals::Motors::CIN2, HIGH);
        digitalWrite(Peripherals::Motors::DIN1, LOW);
        digitalWrite(Peripherals::Motors::DIN2, HIGH);
    }

    auto left() -> void
    {
        //digitalWrite(Peripherals::Motors::STBY, HIGH);
        //digitalWrite(Peripherals::Motors::PWM, HIGH);
        digitalWrite(Peripherals::Motors::AIN1, HIGH);
        digitalWrite(Peripherals::Motors::AIN2, LOW);
        digitalWrite(Peripherals::Motors::BIN1, HIGH);
        digitalWrite(Peripherals::Motors::BIN2, LOW);
        digitalWrite(Peripherals::Motors::CIN1, LOW);
        digitalWrite(Peripherals::Motors::CIN2, HIGH);
        digitalWrite(Peripherals::Motors::DIN1, LOW);
        digitalWrite(Peripherals::Motors::DIN2, HIGH);
    }

    auto right() -> void
    {
        //digitalWrite(Peripherals::Motors::STBY, HIGH);
        //digitalWrite(Peripherals::Motors::PWM, HIGH);
        digitalWrite(Peripherals::Motors::AIN1, LOW);
        digitalWrite(Peripherals::Motors::AIN2, HIGH);
        digitalWrite(Peripherals::Motors::BIN1, LOW);
        digitalWrite(Peripherals::Motors::BIN2, HIGH);
        digitalWrite(Peripherals::Motors::CIN1, HIGH);
        digitalWrite(Peripherals::Motors::CIN2, LOW);
        digitalWrite(Peripherals::Motors::DIN1, HIGH);
        digitalWrite(Peripherals::Motors::DIN2, LOW);
    }

    auto stop() -> void
    {
        //digitalWrite(Peripherals::Motors::STBY, HIGH);
        //digitalWrite(Peripherals::Motors::PWM, LOW);
        digitalWrite(Peripherals::Motors::AIN1, LOW);
        digitalWrite(Peripherals::Motors::AIN2, LOW);
        digitalWrite(Peripherals::Motors::BIN1, LOW);
        digitalWrite(Peripherals::Motors::BIN2, LOW);
        digitalWrite(Peripherals::Motors::CIN1, LOW);
        digitalWrite(Peripherals::Motors::CIN2, LOW);
        digitalWrite(Peripherals::Motors::DIN1, LOW);
        digitalWrite(Peripherals::Motors::DIN2, LOW);
    }
} // namespace Motors