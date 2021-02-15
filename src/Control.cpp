
#include <Arduino.h>
#include <SPIFFS.h>

#include <cstdint>
#include <esp_log.h>

#include "Control.hpp"
#include "Motors.hpp"
#include "Neural.hpp"
#include "Sensors.hpp"

namespace Control
{
    namespace
    {
        auto modeValue{Mode::MANUAL};
        auto autoValue{Auto::STOP};
        auto manualValue{Manual::STOP};
        auto actionTimer{0UL};
        auto manualTimeoutTimer{0UL};

        auto inputs() -> std::array<float, 7>
        {
            auto inputs{std::array<float, 7>{}};

            const auto distances{Sensors::distances()};
            const auto colors{Sensors::colors()};

            inputs[0] = distances[0].second;
            inputs[1] = distances[1].second;
            inputs[2] = distances[2].second;
            inputs[3] = distances[3].second;
            inputs[4] = distances[4].second;
            inputs[5] = distances[5].second;
            //inputs[6] = colors;

            return inputs;
        }

        auto move(Manual manualValue) -> void
        {
            switch (manualValue)
            {
                case Manual::MOVE_FORWARD:
                {
                    Motors::forward();
                    break;
                }
                case Manual::MOVE_BACKWARD:
                {
                    Motors::backward();
                    break;
                }
                case Manual::ROTATE_LEFT:
                {
                    Motors::left();
                    break;
                }
                case Manual::ROTATE_RIGHT:
                {
                    Motors::right();
                    break;
                }
                default:
                {
                    Motors::stop();
                    break;
                }
            }
        }

        auto manual(uint64_t syncTimer) -> void
        {
            if (Control::manualTimeoutTimer == 0)
            {
                Control::manualTimeoutTimer = syncTimer;
            }
            if (syncTimer - Control::manualTimeoutTimer >= 100UL)
            {
                Control::move(Manual::STOP);
            }
            else
            {
                Control::move(Control::manualValue);
            }
        }

        auto automatic(uint64_t syncTimer) -> void
        {
            switch (Control::autoValue)
            {
                case Auto::START:
                {
                    auto inputs{Control::inputs()};

                    for (auto n{0}; n < inputs.size(); ++n)
                    {
                        if (std::isnan(inputs[n]) or std::isinf(inputs[n]) or inputs[n] > 2.0f)
                        {
                            inputs[n] = 2.0f;
                        }
                    }

                    const auto outputs{Neural::inference(inputs)};

                    auto max{0};
                    for (auto n{1}; n < outputs.size(); ++n)
                    {
                        if (std::abs(outputs[n]) > std::abs(outputs[max]))
                        {
                            max = n;
                        }
                    }

                    Control::move(static_cast<Manual>(max));
                    break;
                }
                default:
                {
                    Control::move(Manual::STOP);
                    break;
                }
            }
        }

    } // namespace

    auto init() -> void
    {
        log_d("begin");

        // Nothing

        log_d("end");
    }

    auto process(uint64_t syncTimer) -> void
    {
        if (syncTimer - Control::actionTimer >= 30UL)
        {
            Control::actionTimer = syncTimer;

            if (Control::modeValue == Mode::MANUAL)
            {
                Control::manual(syncTimer);
            }
            else if (Control::modeValue == Mode::AUTO)
            {
                Control::automatic(syncTimer);
            }
        }
    }

    auto mode(Mode modeValue) -> void
    {
        log_d("mode = %d", static_cast<int>(modeValue));
        Control::modeValue = modeValue;
        Control::autoValue = Auto::STOP;
        Control::manualValue = Manual::STOP;
    }

    auto action(Manual manualValue) -> void
    {
        log_d("action(manual) = %d", static_cast<int>(manualValue));
        Control::manualValue = manualValue;
        Control::manualTimeoutTimer = 0;
    }

    auto action(Auto autoValue) -> void
    {
        log_d("action(auto) = %d", static_cast<int>(autoValue));
        Control::autoValue = autoValue;
    }
} // namespace Control