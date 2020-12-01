#pragma once

namespace Display
{
    auto init() -> void;
    auto process() -> void;
    auto printf(int16_t x, int16_t y, const char *format, ...) -> void;
} // namespace Display