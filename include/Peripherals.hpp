#pragma once

#include <array>

namespace Peripherals
{
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

        static constexpr auto XSHUT{std::array<int, 6>{14, 13, 12, 11, 10, 9}};
        static constexpr auto ANGLES{std::array<int, 6>{0, 30, 60, 90, 180, 270}};
    } // namespace Distances
    namespace Color
    {
        enum
        {
            SDA = 5,
            SCL = 6
        };
    }
} // namespace Peripherals