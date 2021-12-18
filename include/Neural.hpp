#pragma once

#include <array>
#include <string>

namespace Neural
{
    auto init() -> void;
    auto inference(const std::array<float, 6> &inputs) -> std::array<float, 5>;
} // namespace Neural