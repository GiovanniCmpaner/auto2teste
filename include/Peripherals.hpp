#pragma once

#include <array>

#include <Arduino.h>
#include <Wire.h>

namespace Peripherals
{
    auto init() -> void;

    namespace Motors
    {
        enum
        {
            STBY = 34,
            PWM = 21,
            AIN1 = 33,
            AIN2 = 26,
            BIN1 = 35,
            BIN2 = 36,
            CIN1 = 42,
            CIN2 = 41,
            DIN1 = 40,
            DIN2 = 39
        };

    }
    namespace Distances
    {
        enum
        {
            SDA = 15,
            SCL = 16
        };
        static constexpr auto &I2C{Wire};
        static constexpr auto XSHUT{std::array<int, 6>{10, 11, 12, 14, 13, 9}};
        static constexpr auto ANGLES{std::array<int, 6>{-90, -33, 0, +33, +90, 180}};
    } // namespace Distances
    namespace Color
    {
        enum
        {
            SDA = 5,
            SCL = 6
        };
        static constexpr auto &I2C{Wire1};
    } // namespace Color
    namespace GyroAccel
    {
        enum
        {
            SDA = 5,
            SCL = 6
        };
        static constexpr auto &I2C{Wire1};
    } // namespace GyroAccel
} // namespace Peripherals