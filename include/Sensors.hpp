#pragma once

#include <array>
#include <utility>
#include <vector>

#include <Arduino.h>
#include <ArduinoJson.hpp>

enum class Color
{
    BLACK = 0,
    RED,
    GREEN,
    BLUE,
    YELLOW,
    MAGENTA,
    CYAN,
    WHITE
};

auto text(Color color) -> const char *;

namespace Sensors
{
    static constexpr auto angleUnit{"deg"};
    static constexpr auto distanceUnit{"m"};
    static constexpr auto rotationUnit{"rad/s"};
    static constexpr auto accelerationUnit{"m/s^2"};
    static constexpr auto magneticUnit{"uT"};
    static constexpr auto temperatureUnit{"degC"};
    static constexpr auto batteryUnit{"%"};

    auto init() -> void;
    auto process(uint64_t syncTimer) -> void;
    auto resetOffset() -> void;
    auto debug() -> void;
    auto serialize(ArduinoJson::JsonVariant &json) -> void;

    auto calibrateGyroscope() -> bool;
    auto calibrateAccelerometer() -> bool;
    auto calibrateMagnetometer() -> bool;

    auto distances() -> std::array<std::pair<int, float>, 6>;
    auto colors() -> std::array<float, 4>;
    auto rotation() -> std::array<float, 3>;
    auto acceleration() -> std::array<float, 3>;
    auto magnetic() -> std::array<float, 3>;
    auto temperature() -> float;
    auto battery() -> float;
} // namespace Sensors