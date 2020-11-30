#pragma once

namespace Motors
{
    enum class Move
    {
        STOP = 0,
        MOVE_FORWARD,
        MOVE_BACKWARD,
        ROTATE_LEFT,
        ROTATE_RIGHT
    };

    auto init() -> void;
    auto process() -> void;

    auto move(Move moveValue, float speedPercent = 100.0f) -> void;
} // namespace Motors