#include <Arduino.h>
#include <esp_log.h>

#include "Configuration.hpp"
#include "Control.hpp"
#include "Display.hpp"
#include "Motors.hpp"
#include "Neural.hpp"
#include "Peripherals.hpp"
#include "Sensors.hpp"
#include "WebInterface.hpp"

void scan()
{
    Wire1.begin();

    while (1)
    {
        byte error, address;
        int nDevices;
        Serial.println("Scanning...");
        nDevices = 0;
        for (address = 1; address < 127; address++)
        {
            Wire1.beginTransmission(address);
            error = Wire1.endTransmission();
            if (error == 0)
            {
                Serial.print("I2C device found at address 0x");
                if (address < 16)
                {
                    Serial.print("0");
                }
                Serial.println(address, HEX);
                nDevices++;
            }
            else if (error == 4)
            {
                Serial.print("Unknow error at address 0x");
                if (address < 16)
                {
                    Serial.print("0");
                }
                Serial.println(address, HEX);
            }
        }
        if (nDevices == 0)
        {
            Serial.println("No I2C devices found\n");
        }
        else
        {
            Serial.println("done\n");
        }
        delay(5000);
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    log_d("begin");

    Peripherals::init();

    //scan();

    Display::init();
    Display::printf("initializing");

    Configuration::init();
    Sensors::init();
    Motors::init();
    Neural::init();
    Control::init();
    WebInterface::init();

    Display::printf("ready");

    log_d("end");
}

void loop()
{
    const auto syncTimer{millis()};
    Display::process(syncTimer);
    Sensors::process(syncTimer);
    Motors::process(syncTimer);
    Neural::process(syncTimer);
    Control::process(syncTimer);
    WebInterface::process(syncTimer);
    delay(1); // Necessário para o ESP TCP Async
}