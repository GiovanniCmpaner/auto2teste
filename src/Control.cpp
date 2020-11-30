
#include <Arduino.h>
#include <SPIFFS.h>

#include <cstdint>
#include <esp_log.h>

//#undef PI
//#include <tiny_dnn/tiny_dnn.h>

#include "Control.hpp"
#include "Motors.hpp"
#include "Sensors.hpp"

namespace Control
{
    static auto state{0};
    static auto direction{0};

    static auto baseDistance{NAN};
    static auto leftDistance{NAN};
    static auto rightDistance{NAN};

    static auto currentTime{0UL};
    static auto currentDelay{0UL};

    static auto load() -> void
    {
        //        log_d("begin");
        //
        //        if (SPIFFS.exists("/neural_network_loading.json"))
        //        {
        //            log_e("file error");
        //
        //            SPIFFS.remove("/neural_network_loading.json");
        //        }
        //
        //        if (SPIFFS.exists("/neural_network.json"))
        //        {
        //            log_d("file exists");
        //
        //            SPIFFS.rename("/neural_network.json", "/neural_network_loading.json");
        //
        //            using namespace tiny_dnn;
        //            auto net{network<sequential>{}};
        //            net.load("/spiffs/neural_network_loading.json", content_type::weights_and_model, file_format::json);
        //
        //            assert(net.in_data_size() == 13);
        //            assert(net.out_data_size() == 4);
        //
        //            SPIFFS.rename("/neural_network_loading.json", "/neural_network.json");
        //        }
        //
        //        log_d("end");
    }

    auto init() -> void
    {
        log_d("begin");

        Control::load();

        log_d("end");
    }

    auto process() -> void
    {
        if (Control::state == 0)
        {
            // NORMAL
        }
        else if (Control::state == 1)
        {
            Motors::stop();
            Motors::speed(25.0f);

            Control::state = 2;
            log_d("state = 2");

            Control::baseDistance = Sensors::distances()[0];
            Control::leftDistance = NAN;
            Control::rightDistance = NAN;
            Control::direction = 0;
            Control::currentTime = 0;
            Control::currentDelay = 2000UL;
        }
        else if (Control::state == 2)
        {
            if (millis() - Control::currentTime > Control::currentDelay)
            {
                Motors::stop();

                if (Control::direction == 0)
                {
                    if (isnan(Control::leftDistance))
                    {
                        Control::direction = -1;
                        Motors::left();
                    }
                    else if (isnan(Control::rightDistance))
                    {
                        Control::direction = +1;
                        Motors::right();
                    }
                    else
                    {
                        if (Control::leftDistance - Control::rightDistance > +0.005 and Control::currentDelay > 1)
                        {
                            Control::state = 3;
                            log_d("state = 3");
                            Control::currentDelay /= 2;
                            Control::rightDistance = NAN;
                            Control::direction = +1;
                            Motors::right();
                        }
                        else if (Control::rightDistance - Control::leftDistance > +0.005 and Control::currentDelay > 1)
                        {
                            Control::state = 3;
                            log_d("state = 3");
                            Control::currentDelay /= 2;
                            Control::leftDistance = NAN;
                            Control::direction = -1;
                            Motors::left();
                        }
                        else
                        {
                            Control::state = 0;
                            Motors::speed(100.0f);
                            log_d("state = 0");

                            const auto distances{Sensors::distances()};

                            log_d("done = [0] %.3f / [-30] %.3f / [+30] %.3f", distances.at(0), distances.at(-30), distances.at(+30));
                        }
                    }
                }
                else if (Control::direction == -1)
                {
                    Control::leftDistance = Sensors::distances()[0];
                    Control::direction = 0;
                    Motors::right();
                }
                else if (Control::direction == +1)
                {
                    Control::rightDistance = Sensors::distances()[0];
                    Control::direction = 0;
                    Motors::left();
                }

                Control::currentTime = millis();
            }
        }
        else if (Control::state == 3)
        {
            if (millis() - Control::currentTime > Control::currentDelay)
            {
                Motors::stop();

                Control::state = 2;
                log_d("state = 2");
                Control::baseDistance = Sensors::distances()[0];
                Control::leftDistance = NAN;
                Control::rightDistance = NAN;
                Control::direction = 0;
                Control::currentTime = 0;
            }
        }
    }

    auto calibrateDistances() -> void
    {
        if (Control::state == 0)
        {
            Control::state = 1;
        }
    }
} // namespace Control