#pragma once

namespace Motors
{
    auto init() -> void;

    auto forward() -> void;
    auto backward() -> void;
    auto left() -> void;
    auto right() -> void;
    auto stop() -> void;

    auto speed(float speedPercent) -> void;

} // namespace Motors