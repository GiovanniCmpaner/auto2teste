#pragma once

#include <utility>
#include <array>
#include <map>

#include <Arduino.h>
#include <ArduinoJson.hpp>

namespace Sensors
{
    static constexpr auto angleUnit{"deg"};
    static constexpr auto distanceUnit{"m"};
    static constexpr auto rotationUnit{"rad/s"};
    static constexpr auto accelerationUnit{"m/s^2"};
    static constexpr auto temperatureUnit{"degC"};

    auto init() -> void;
    auto process() -> void;
    auto resetReference() -> void;
    auto print() -> void;
    auto serialize(ArduinoJson::JsonVariant &json) -> void;

    auto distances() -> std::map<int, float>;
    auto color() -> std::array<uint16_t, 3>;
    auto rotation() -> std::array<float, 3>;
    auto acceleration() -> std::array<float, 3>;
    auto temperature() -> float;
} // namespace Sensors