
#include <Arduino.h>
#include <SPIFFS.h>

#include <cstdint>
#include <esp_log.h>

#include "Control.hpp"
#include "Motors.hpp"
#include "Sensors.hpp"

namespace Control
{
    namespace
    {
        auto modeValue{Mode::MANUAL};
        auto autoValue{Auto::STOP};
        auto manualValue{Manual::STOP};

        //auto inputs() -> std::vector<float>
        //{
        //    auto inputs{std::vector<float>{}};
        //    {
        //        const auto distances{Sensors::distances()};
        //        for (const auto [angle, distance] : distances)
        //        {
        //            inputs.emplace_back(distance);
        //        }
        //    }
        //    {
        //        const auto colors{Sensors::colors()};
        //        for (auto color : colors)
        //        {
        //            inputs.emplace_back(color);
        //        }
        //    }
        //    return inputs;
        //}
    } // namespace

    auto init() -> void
    {
        log_d("begin");

        // Nothing

        log_d("end");
    }

    auto process(uint64_t syncTimer) -> void
    {
        if (modeValue == Mode::MANUAL)
        {
            Motors::move(static_cast<Move>(manualValue));
        }
        else if (modeValue == Mode::AUTO)
        {
            // Read the predicted y value from the model's output tensor
            //{
            //    auto maxIndex{0};
            //    for (auto n{1}; n < 5; ++n)
            //    {
            //        if (output->data.f[maxIndex] < output->data.f[n])
            //        {
            //            maxIndex = n;
            //        }
            //    }
            //    Motors::move(static_cast<Move>(maxIndex));
            //}
        }
    }

    auto mode(Mode modeValue) -> void
    {
        Control::modeValue = modeValue;
        Control::autoValue = Auto::STOP;
    }

    auto action(Manual manualValue) -> void
    {
        Control::manualValue = manualValue;
    }

    auto action(Auto autoValue) -> void
    {
        Control::autoValue = autoValue;
    }
} // namespace Control