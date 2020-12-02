#pragma once

namespace Display
{
    auto init() -> void;
    auto process() -> void;
    auto printf(const char *format, ...) -> void;
} // namespace Display