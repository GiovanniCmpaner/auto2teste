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
    GRAY,
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

    auto init() -> void;
    auto process() -> void;
    auto resetOffset() -> void;
    auto print() -> void;
    auto serialize(ArduinoJson::JsonVariant &json) -> void;

    auto distances() -> std::array<std::pair<int, float>, 6>;
    auto color() -> std::array<uint16_t, 3>;
    auto rotation() -> std::array<float, 3>;
    auto acceleration() -> std::array<float, 3>;
    auto magnetic() -> std::array<float, 3>;
    auto temperature() -> float;
} // namespace Sensors