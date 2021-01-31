#pragma once

#include <string>
#include <vector>

namespace Neural
{
    auto init() -> void;
    auto process(uint64_t syncTimer) -> void;
    auto inputs(const std::vector<float> &inputValues) -> bool;
    auto outputs(std::vector<float> *outputValues) -> bool;
} // namespace Neural