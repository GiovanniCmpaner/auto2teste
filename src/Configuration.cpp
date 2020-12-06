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
    {{{0.0f, 0.0f, 0.0f}},
     {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
     {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}},
    {true,
     {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED},
     {192, 168, 1, 210},
     {255, 255, 255, 0},
     {192, 168, 1, 1},
     80,
     "WORKGROUP",
     "49WNN7F3CD@22"},
    {true,
     {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED},
     {192, 168, 1, 210},
     {255, 255, 255, 0},
     {192, 168, 1, 1},
     80,
     "Auto2",
     "4ut@Con7r0L",
     30}};

auto Configuration::init() -> void
{
    log_d("begin");

    SPIFFS.begin(true);

    Configuration::load(&cfg);

    log_d("end");
}

auto Configuration::serialize(ArduinoJson::JsonVariant &json) const -> void
{
    {
        auto calibration{json["calibration"]};
        {
            auto gyroscope{calibration["gyroscope"]};
            for (auto n : this->calibration.gyroscope.bias)
            {
                gyroscope["bias"].add(n);
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
        log_d("file not found");
    }
    else
    {
        auto file{SPIFFS.open("/configuration.json", FILE_READ)};
        if (not file)
        {
            log_e("file error");
        }
        else
        {
            auto doc{ArduinoJson::DynamicJsonDocument{3072}};
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
        log_e("file error");
    }
    else
    {
        auto doc{ArduinoJson::DynamicJsonDocument{3072}};
        auto json{doc.as<ArduinoJson::JsonVariant>()};

        cfg.serialize(json);

        ArduinoJson::serializeJsonPretty(doc, file);
        file.close();
    }

    log_d("end");
}

Configuration cfg{};