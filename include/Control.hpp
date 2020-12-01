#pragma once

namespace Control
{
    enum class Mode
    {
        MANUAL = 0,
        AUTOMATIC
    };

    auto init() -> void;
    auto process() -> void;
    auto mode(Mode modeValue) -> void;
}