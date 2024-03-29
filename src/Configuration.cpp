#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <FS.h>
#include <FastCRC.h>
#include <SPIFFS.h>

#include <cstdio>
#include <cstdlib>
#include <esp_log.h>
#include <string>

#include "Configuration.hpp"
#include "Peripherals.hpp"

static const Configuration defaultCfg{
    .calibration = {
        .motor = {.speed = 0.70f},
        .battery = {-166.67f, 0.0417f},
        .gyroscope = {.bias = {0.0f, 0.0f, 0.0f}, .factor = {1.0f, 1.0f, 1.0f}},
        .accelerometer = {.bias = {0.0f, 0.0f, 0.0f}, .factor = {1.0f, 1.0f, 1.0f}},
        .magnetometer = {.bias = {0.0f, 0.0f, 0.0f}, .factor = {1.0f, 1.0f, 1.0f}},
        .distance = {.bias = {-0.020738f, -0.023718f, -0.025331f, -0.043292f, -0.015682f, -0.027025f}, .factor = {1.003176f, 0.968107f, 0.984809f, 0.987616f, 0.981429f, 0.986715f}},
        .color = {.target = 520.0f, .threshold = {115.0f, 154.0f, 200.0f}}},
    .station = {.enabled = true, .mac = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}, .ip = {192, 168, 1, 210}, .netmask = {255, 255, 255, 0}, .gateway = {192, 168, 1, 1}, .port = 80, .user = "WORKGROUP", .password = "49WNN7F3CD@22"},
    .accessPoint = {.enabled = false, .mac = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}, .ip = {192, 168, 1, 210}, .netmask = {255, 255, 255, 0}, .gateway = {192, 168, 1, 1}, .port = 80, .user = "Auto2", .password = "4ut@Con7r0L", .duration = 30}};

auto Configuration::init() -> void
{
    log_d("begin");

    Configuration::load(&cfg);

    log_d("end");
}

auto Configuration::serialize(ArduinoJson::JsonVariant &json) const -> void
{
    {
        auto calibration{json["calibration"]};
        {
            auto motor{calibration["motor"]};

            motor["speed"] = constrain(this->calibration.motor.speed, 0.01f, 1.00f);
        }
        {
            auto battery{calibration["battery"]};

            battery["bias"] = this->calibration.battery.bias;
            battery["factor"] = this->calibration.battery.factor;
        }
        {
            auto gyroscope{calibration["gyroscope"]};
            for (auto n : this->calibration.gyroscope.bias)
            {
                gyroscope["bias"].add(n);
            }
            for (auto n : this->calibration.gyroscope.factor)
            {
                gyroscope["factor"].add(n);
            }
        }
        {
            auto accelerometer{calibration["accelerometer"]};
            for (auto n : this->calibration.accelerometer.bias)
            {
                accelerometer["bias"].add(n);
            }
            for (auto n : this->calibration.accelerometer.factor)
            {
                accelerometer["factor"].add(n);
            }
        }
        {
            auto magnetometer{calibration["magnetometer"]};
            for (auto n : this->calibration.magnetometer.bias)
            {
                magnetometer["bias"].add(n);
            }
            for (auto n : this->calibration.magnetometer.factor)
            {
                magnetometer["factor"].add(n);
            }
        }
        {
            auto distance{calibration["distance"]};
            for (auto n : this->calibration.distance.bias)
            {
                distance["bias"].add(n);
            }
            for (auto n : this->calibration.distance.factor)
            {
                distance["factor"].add(n);
            }
        }
        {
            auto color{calibration["color"]};

            color["target"] = this->calibration.color.target;

            for (auto n : this->calibration.color.threshold)
            {
                color["threshold"].add(n);
            }
        }
    }
    {
        auto accessPoint{json["access_point"]};

        accessPoint["enabled"] = this->accessPoint.enabled;
        for (auto n : this->accessPoint.mac)
        {
            accessPoint["mac"].add(n);
        }
        for (auto n : this->accessPoint.ip)
        {
            accessPoint["ip"].add(n);
        }
        for (auto n : this->accessPoint.netmask)
        {
            accessPoint["netmask"].add(n);
        }
        for (auto n : this->accessPoint.gateway)
        {
            accessPoint["gateway"].add(n);
        }
        accessPoint["port"] = this->accessPoint.port;
        accessPoint["user"] = this->accessPoint.user;
        accessPoint["password"] = this->accessPoint.password;
        accessPoint["duration"] = this->accessPoint.duration;
    }
    {
        auto station{json["station"]};

        station["enabled"] = this->station.enabled;
        for (auto n : this->station.mac)
        {
            station["mac"].add(n);
        }
        for (auto n : this->station.ip)
        {
            station["ip"].add(n);
        }
        for (auto n : this->station.netmask)
        {
            station["netmask"].add(n);
        }
        for (auto n : this->station.gateway)
        {
            station["gateway"].add(n);
        }
        station["port"] = this->station.port;
        station["user"] = this->station.user;
        station["password"] = this->station.password;
    }
}

auto Configuration::deserialize(const ArduinoJson::JsonVariant &json) -> void
{
    {
        const auto calibration{json["calibration"]};
        {
            const auto motor{calibration["motor"]};
            {
                const auto speed{motor["speed"]};
                if (speed.is<float>())
                {
                    this->calibration.motor.speed = constrain(speed.as<float>(), 0.01f, 1.00f);
                }
            }
        }
        {
            const auto battery{calibration["battery"]};
            {
                const auto bias{battery["bias"]};
                if (bias.is<float>())
                {
                    this->calibration.battery.bias = bias.as<float>();
                }
            }
            {
                const auto factor{battery["factor"]};
                if (factor.is<float>())
                {
                    this->calibration.battery.factor = factor.as<float>();
                }
            }
        }
        {
            const auto gyroscope{calibration["gyroscope"]};
            {
                const auto bias{gyroscope["bias"]};
                if (bias.is<ArduinoJson::JsonArray>() and bias.size() == this->calibration.gyroscope.bias.size())
                {
                    for (auto i{0}; i < this->calibration.gyroscope.bias.size(); ++i)
                    {
                        this->calibration.gyroscope.bias[i] = bias[i].as<float>();
                    }
                }
            }
            {
                const auto factor{gyroscope["factor"]};
                if (factor.is<ArduinoJson::JsonArray>() and factor.size() == this->calibration.gyroscope.factor.size())
                {
                    for (auto i{0}; i < this->calibration.gyroscope.factor.size(); ++i)
                    {
                        this->calibration.gyroscope.factor[i] = factor[i].as<float>();
                    }
                }
            }
        }
        {
            const auto accelerometer{calibration["accelerometer"]};
            {
                const auto bias{accelerometer["bias"]};
                if (bias.is<ArduinoJson::JsonArray>() and bias.size() == this->calibration.accelerometer.bias.size())
                {
                    for (auto i{0}; i < this->calibration.accelerometer.bias.size(); ++i)
                    {
                        this->calibration.accelerometer.bias[i] = bias[i].as<float>();
                    }
                }
            }
            {
                const auto factor{accelerometer["factor"]};
                if (factor.is<ArduinoJson::JsonArray>() and factor.size() == this->calibration.accelerometer.factor.size())
                {
                    for (auto i{0}; i < this->calibration.accelerometer.factor.size(); ++i)
                    {
                        this->calibration.accelerometer.factor[i] = factor[i].as<float>();
                    }
                }
            }
        }
        {
            const auto magnetometer{calibration["magnetometer"]};
            {
                const auto bias{magnetometer["bias"]};
                if (bias.is<ArduinoJson::JsonArray>() and bias.size() == this->calibration.magnetometer.bias.size())
                {
                    for (auto i{0}; i < this->calibration.magnetometer.bias.size(); ++i)
                    {
                        this->calibration.magnetometer.bias[i] = bias[i].as<float>();
                    }
                }
            }
            {
                const auto factor{magnetometer["factor"]};
                if (factor.is<ArduinoJson::JsonArray>() and factor.size() == this->calibration.magnetometer.factor.size())
                {
                    for (auto i{0}; i < this->calibration.magnetometer.factor.size(); ++i)
                    {
                        this->calibration.magnetometer.factor[i] = factor[i].as<float>();
                    }
                }
            }
        }
        {
            const auto distance{calibration["distance"]};
            {
                const auto bias{distance["bias"]};
                if (bias.is<ArduinoJson::JsonArray>() and bias.size() == this->calibration.distance.bias.size())
                {
                    for (auto i{0}; i < this->calibration.distance.bias.size(); ++i)
                    {
                        this->calibration.distance.bias[i] = bias[i].as<float>();
                    }
                }
            }
            {
                const auto factor{distance["factor"]};
                if (factor.is<ArduinoJson::JsonArray>() and factor.size() == this->calibration.distance.factor.size())
                {
                    for (auto i{0}; i < this->calibration.distance.factor.size(); ++i)
                    {
                        this->calibration.distance.factor[i] = factor[i].as<float>();
                    }
                }
            }
        }
        {
            const auto color{calibration["color"]};
            {
                const auto target{color["target"]};
                if (target.is<float>())
                {
                    this->calibration.color.target = target.as<float>();
                }
            }
            {
                const auto threshold{color["threshold"]};
                if (threshold.is<ArduinoJson::JsonArray>() and threshold.size() == this->calibration.color.threshold.size())
                {
                    for (auto i{0}; i < this->calibration.color.threshold.size(); ++i)
                    {
                        this->calibration.color.threshold[i] = threshold[i].as<float>();
                    }
                }
            }
        }
    }
    {
        const auto accessPoint{json["access_point"]};
        // IGNORE this->accessPoint.mac
        {
            const auto enabled{accessPoint["enabled"]};
            if (enabled.is<bool>())
            {
                this->accessPoint.enabled = enabled.as<bool>();
            }
        }
        {
            const auto ip{accessPoint["ip"]};
            if (ip.is<ArduinoJson::JsonArray>() and ip.size() == this->accessPoint.ip.size())
            {
                for (auto i{0}; i < this->accessPoint.ip.size(); ++i)
                {
                    this->accessPoint.ip[i] = ip[i].as<uint8_t>();
                }
            }
        }
        {
            const auto netmask{accessPoint["netmask"]};
            if (netmask.is<ArduinoJson::JsonArray>() and netmask.size() == this->accessPoint.netmask.size())
            {
                for (auto i{0}; i < this->accessPoint.netmask.size(); ++i)
                {
                    this->accessPoint.netmask[i] = netmask[i].as<uint8_t>();
                }
            }
        }
        {
            const auto gateway{accessPoint["gateway"]};
            if (gateway.is<ArduinoJson::JsonArray>() and gateway.size() == this->accessPoint.gateway.size())
            {
                for (auto i{0}; i < this->accessPoint.gateway.size(); ++i)
                {
                    this->accessPoint.gateway[i] = gateway[i].as<uint8_t>();
                }
            }
        }
        {
            const auto port{accessPoint["port"]};
            if (port.is<uint16_t>())
            {
                this->accessPoint.port = port.as<uint16_t>();
            }
        }
        {
            const auto user{accessPoint["user"]};
            if (user.is<std::string>())
            {
                this->accessPoint.user = user.as<std::string>();
            }
        }
        {
            const auto password{accessPoint["password"]};
            if (password.is<std::string>())
            {
                this->accessPoint.password = password.as<std::string>();
            }
        }
        {
            const auto duration{accessPoint["duration"]};
            if (duration.is<uint16_t>())
            {
                this->accessPoint.duration = duration.as<uint16_t>();
            }
        }
    }
    {
        const auto station{json["station"]};
        // IGNORE this->station.mac
        {
            const auto enabled{station["enabled"]};
            if (enabled.is<bool>())
            {
                this->station.enabled = enabled.as<bool>();
            }
        }
        {
            const auto ip{station["ip"]};
            if (ip.is<ArduinoJson::JsonArray>() and ip.size() == this->station.ip.size())
            {
                for (auto i{0}; i < this->station.ip.size(); ++i)
                {
                    this->station.ip[i] = ip[i].as<uint8_t>();
                }
            }
        }
        {
            const auto netmask{station["netmask"]};
            if (netmask.is<ArduinoJson::JsonArray>() and netmask.size() == this->station.netmask.size())
            {
                for (auto i{0}; i < this->station.netmask.size(); ++i)
                {
                    this->station.netmask[i] = netmask[i].as<uint8_t>();
                }
            }
        }
        {
            const auto gateway{station["gateway"]};
            if (gateway.is<ArduinoJson::JsonArray>() and gateway.size() == this->station.gateway.size())
            {
                for (auto i{0}; i < this->station.gateway.size(); ++i)
                {
                    this->station.gateway[i] = gateway[i].as<uint8_t>();
                }
            }
        }
        {
            const auto port{station["port"]};
            if (port.is<uint16_t>())
            {
                this->station.port = port.as<uint16_t>();
            }
        }
        {
            const auto user{station["user"]};
            if (user.is<std::string>())
            {
                this->station.user = user.as<std::string>();
            }
        }
        {
            const auto password{station["password"]};
            if (password.is<std::string>())
            {
                this->station.password = password.as<std::string>();
            }
        }
    }
}

auto Configuration::load(Configuration *cfg) -> void
{
    log_d("begin");

    *cfg = defaultCfg;

    if (not SPIFFS.exists("/configuration.json"))
    {
        log_d("config file not found");
    }
    else
    {
        auto file{SPIFFS.open("/configuration.json", FILE_READ)};
        if (not file)
        {
            log_e("config file error");
        }
        else
        {
            auto doc{ArduinoJson::DynamicJsonDocument{4096}};
            auto err{ArduinoJson::deserializeJson(doc, file)};
            file.close();

            if (err != ArduinoJson::DeserializationError::Ok)
            {
                log_d("json error = %s", err.c_str());
            }
            else
            {
                auto json{doc.as<ArduinoJson::JsonVariant>()};
                cfg->deserialize(json);
            }
        }
    }

    if (not cfg->accessPoint.enabled and not cfg->station.enabled)
    {
        cfg->accessPoint.enabled = true;
        cfg->station.enabled = true;
    }

    Configuration::save(*cfg);

    log_d("end");
}

auto Configuration::save(const Configuration &cfg) -> void
{
    log_d("begin");

    auto file{SPIFFS.open("/configuration.json", FILE_WRITE)};
    if (not file)
    {
        log_e("config file error");
    }
    else
    {
        auto doc{ArduinoJson::DynamicJsonDocument{4096}};
        auto json{doc.as<ArduinoJson::JsonVariant>()};

        cfg.serialize(json);

        ArduinoJson::serializeJsonPretty(doc, file);
        file.close();
    }

    log_d("end");
}

Configuration cfg{};