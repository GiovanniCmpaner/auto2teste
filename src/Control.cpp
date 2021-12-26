
#include <Arduino.h>
#include <SPIFFS.h>

#include <array>
#include <cstdint>
#include <esp_log.h>
#include <functional>
#include <iomanip>
#include <sstream>

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
        auto captureEnabled{false};
        auto captureReading{false};
        auto captureFile{fs::File{}};

        auto inputs() -> std::array<float, 6>
        {
            auto inputs{std::array<float, 6>{}};

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
                Control::Capture::save();
            }
        }

        auto automatic(uint64_t syncTimer) -> void
        {
            switch (Control::autoValue)
            {
                case Auto::START:
                {
                    auto inputs{Control::inputs()};

                    const auto outputs{Neural::inference(inputs)};

                    auto max{0};
                    for (auto n{1}; n < outputs.size(); ++n)
                    {
                        if (std::abs(outputs[n]) > std::abs(outputs[max]))
                        {
                            max = n;
                        }
                    }

                    //log_d("inputs = %.2f %.2f %.2f %.2f %.2f %.2f", inputs[0], inputs[1], inputs[2], inputs[3], inputs[4], inputs[5]);
                    //log_d("outputs = %.2f %.2f %.2f %.2f %.2f", outputs[0], outputs[1], outputs[2], outputs[3], outputs[4]);

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
        if (syncTimer - Control::actionTimer >= 33UL)
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

    namespace Capture
    {
        auto enable() -> bool
        {
            log_d("capture enable");

            if (Control::captureEnabled or Control::captureReading)
            {
                log_d("capture busy");
                return false;
            }

            Control::captureFile = SPIFFS.open("/capture.bin", FILE_APPEND);
            if (not Control::captureFile)
            {
                log_e("capture file write error");
                return false;
            }

            Control::captureEnabled = true;
            return true;
        }

        auto disable() -> void
        {
            log_d("capture disable");

            if (not Control::captureEnabled)
            {
                log_d("capture not enabled");
                return;
            }

            Control::captureFile.close();
            Control::captureEnabled = false;
        }

        auto clear() -> bool
        {
            log_d("capture clear");

            if (Control::captureEnabled or Control::captureReading)
            {
                log_d("capture busy");
                return false;
            }

            SPIFFS.remove("/capture.bin");
            return true;
        }

        auto save() -> bool
        {
            if (not Control::captureEnabled)
            {
                return false;
            }

            if (Control::manualValue != Manual::STOP)
            {
                const auto inputs{Control::inputs()};
                Control::captureFile.write(reinterpret_cast<const uint8_t *>(&inputs), sizeof(inputs));
                Control::captureFile.write(reinterpret_cast<const uint8_t *>(&Control::manualValue), sizeof(Control::manualValue));
            }
            return true;
        }

        auto beginReadCsv() -> bool
        {
            if (Control::captureEnabled or Control::captureReading)
            {
                log_d("capture busy");
                return false;
            }

            Control::captureFile = SPIFFS.open("/capture.bin", FILE_READ);
            if (not Control::captureFile)
            {
                log_e("capture file read error");
                return false;
            }

            Control::captureReading = true;
            return true;
        }

        auto headerLineCsv(std::string *str) -> bool
        {
            if (not Control::captureReading)
            {
                log_d("capture not reading");
                return false;
            }

            *str = "+33;+90;0;-33;-90;180;stop;forward;backward;left;right;\n";
            return true;
        }

        auto nextLineCsv(std::string *str) -> bool
        {
            if (not Control::captureReading)
            {
                log_d("capture not reading");
                return false;
            }

            auto inputs{std::array<float, 6>{}};
            auto manual{Manual::STOP};
            if (not(Control::captureFile.read(reinterpret_cast<uint8_t *>(&inputs), sizeof(inputs)) and Control::captureFile.read(reinterpret_cast<uint8_t *>(&manual), sizeof(manual))))
            {
                log_d("capture read end");
                return false;
            }

            auto oss{std::ostringstream{}};

            for (auto n{0}; n < 6; ++n)
            {
                oss << std::fixed << std::setprecision(3) << inputs[n] << ';';
            }

            for (auto n{0}; n < 5; ++n)
            {
                oss << (static_cast<int>(manual) == n ? '1' : '0') << ';';
            }

            oss << '\n';

            *str = oss.str();
            return true;
        }

        auto endReadCsv() -> void
        {
            if (not Control::captureReading)
            {
                log_d("capture not reading");
                return;
            }

            Control::captureFile.close();
            Control::captureReading = false;
        }
    } // namespace Capture

} // namespace Control