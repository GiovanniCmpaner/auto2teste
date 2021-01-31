#pragma once

namespace Display
{
    auto init() -> void;
    auto process(uint64_t syncTimer) -> void;
    auto printf(const char *format, ...) -> void;
} // namespace Display