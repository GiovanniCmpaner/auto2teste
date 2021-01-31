#pragma once

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
} // namespace Control