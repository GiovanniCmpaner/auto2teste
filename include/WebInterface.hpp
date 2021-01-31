#pragma once

namespace WebInterface
{
    auto init() -> void;
    auto process(uint64_t syncTimer) -> void;
}