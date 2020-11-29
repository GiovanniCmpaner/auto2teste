#pragma once

#include <utility>
#include <array>
#include <vector>

namespace Distance
{
    auto init() -> void;
    auto process() -> void;
    auto values() -> std::vector<std::pair<int, float>>;
}