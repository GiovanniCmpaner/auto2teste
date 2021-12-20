#pragma once

#include <array>
#include <cstdint>
#include <functional>

#include <Arduino.h>
#include <ArduinoJson.hpp>

enum class Mode
{
    MANUAL = 0,
    AUTO
};

enum class Manual
{
    STOP = 0,
    MOVE_FORWARD,
    MOVE_BACKWARD,
    ROTATE_LEFT,
    ROTATE_RIGHT
};

enum class Auto
{
    STOP = 0,
    START
};

namespace Control
{
    auto init() -> void;
    auto process(uint64_t syncTimer) -> void;

    auto mode(Mode modeValue) -> void;
    auto action(Manual manualValue) -> void;
    auto action(Auto autoValue) -> void;

    namespace Capture
    {
        auto enable() -> bool;
        auto disable() -> void;
        auto clear() -> bool;
        auto save() -> bool;

        auto beginReadCsv() -> bool;
        auto headerLineCsv(std::string *str) -> bool;
        auto nextLineCsv(std::string *str) -> bool;
        auto endReadCsv() -> void;
    } // namespace Capture

} // namespace Control