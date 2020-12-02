#include <Arduino.h>

#include "Motors.hpp"
#include "Peripherals.hpp"

namespace Motors
{
    static auto moveValue{Move::STOP};
    static auto speedPercent{100.0f};

    static auto forward() -> void
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

    static auto backward() -> void
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

    static auto left() -> void
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

    static auto right() -> void
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

    static auto stop() -> void
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

    auto init() -> void
    {
        log_d("begin");

        ledcSetup(0, 10000, 8);
        ledcAttachPin(Peripherals::Motors::PWM, 0);
        ledcWrite(0, 255);

        digitalWrite(Peripherals::Motors::STBY, HIGH);

        log_d("end");
    }

    auto process() -> void
    {
        static auto moveTimer{0UL};
        if (millis() - moveTimer >= 30UL)
        {
            moveTimer = millis();

            switch (Motors::moveValue)
            {
                case Move::MOVE_FORWARD:
                {
                    Motors::forward();
                    break;
                }
                case Move::MOVE_BACKWARD:
                {
                    Motors::backward();
                    break;
                }
                case Move::ROTATE_LEFT:
                {
                    Motors::left();
                    break;
                }
                case Move::ROTATE_RIGHT:
                {
                    Motors::right();
                    break;
                }
                default:
                {
                    Motors::stop();
                    break;
                }
            }
        }
    }

    auto move(Move moveValue, float speedPercent) -> void
    {
        Motors::moveValue = moveValue;
        Motors::speedPercent = speedPercent;

        //ledcWrite(0, speedPercent / 100.0 * 1023.0);
    }

} // namespace Motors