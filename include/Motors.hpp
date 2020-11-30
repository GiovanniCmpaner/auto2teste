#pragma once

namespace Motors
{
    auto init() -> void;
    auto process() -> void;

    auto speed(float percent) -> void;
    auto forward() -> void;
    auto backward() -> void;
    auto left() -> void;
    auto right() -> void;
    auto stop() -> void;
}