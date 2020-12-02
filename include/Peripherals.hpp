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
            SCL = 16,

            ADDRESS = 0x29
        };
        static constexpr auto &I2C{Wire};

        static constexpr auto XSHUT{std::array<int, 6>{14, 13, 12, 11, 10, 9}};
        static constexpr auto ANGLES{std::array<int, 6>{+33, +90, 0, -33, -90, 180}};
    } // namespace Distances

    namespace Color
    {
        enum
        {
            SDA = 5,
            SCL = 6,

            ADDRESS = 0x39
        };
        static constexpr auto &I2C{Wire1};
    } // namespace Color

    namespace GyroAccelMag
    {
        enum
        {
            SDA = 5,
            SCL = 6,

            ADDRESS = 0x68
        };
        static constexpr auto &I2C{Wire1};
    } // namespace GyroAccelMag

    namespace Display
    {
        enum
        {
            SDA = 5,
            SCL = 6,

            WIDTH = 128,
            HEIGHT = 64,

            ADDRESS = 0x3C
        };
        static constexpr auto &I2C{Wire1};
    } // namespace Display

    namespace Battery
    {
        enum
        {
            VIN = 1
        };
    }

    namespace LED
    {
        enum
        {
            CTRL = 8
        };
    }

} // namespace Peripherals