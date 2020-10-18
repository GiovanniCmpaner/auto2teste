#include <Arduino.h>

#include "Peripherals.hpp"

namespace Peripherals
{
    auto init() -> void
    {
        pinMode( Peripherals::Motors::STBY, OUTPUT );
        pinMode( Peripherals::Motors::PWM, OUTPUT );
        pinMode( Peripherals::Motors::AIN1, OUTPUT );
        pinMode( Peripherals::Motors::AIN2, OUTPUT );
        pinMode( Peripherals::Motors::BIN1, OUTPUT );
        pinMode( Peripherals::Motors::BIN2, OUTPUT );
        pinMode( Peripherals::Motors::CIN1, OUTPUT );
        pinMode( Peripherals::Motors::CIN2, OUTPUT );
        pinMode( Peripherals::Motors::DIN1, OUTPUT );
        pinMode( Peripherals::Motors::DIN2, OUTPUT );

        digitalWrite( Peripherals::Motors::STBY, LOW );
        digitalWrite( Peripherals::Motors::PWM, LOW );
        digitalWrite( Peripherals::Motors::AIN1, LOW );
        digitalWrite( Peripherals::Motors::AIN2, LOW );
        digitalWrite( Peripherals::Motors::BIN1, LOW );
        digitalWrite( Peripherals::Motors::BIN2, LOW );
        digitalWrite( Peripherals::Motors::CIN1, LOW );
        digitalWrite( Peripherals::Motors::CIN2, LOW );
        digitalWrite( Peripherals::Motors::DIN1, LOW );
        digitalWrite( Peripherals::Motors::DIN2, LOW );
    }
}