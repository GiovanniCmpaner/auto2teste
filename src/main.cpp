//#include <Arduino.h>
//#include <WiFi.h>
//#include <esp_log.h>
//
//#include "Configuration.hpp"
//#include "Control.hpp"
//#include "Display.hpp"
//#include "Motors.hpp"
//#include "Neural.hpp"
//#include "Peripherals.hpp"
//#include "Sensors.hpp"
//#include "WebInterface.hpp"
//
//void setup()
//{
//    Serial.begin(115200);
//    Serial.setDebugOutput(true);
//
//    log_d("begin");
//
//    Peripherals::init();
//
//    Display::init();
//    Display::printf("initializing");
//
//    Configuration::init();
//    Sensors::init();
//    Motors::init();
//    Neural::init();
//    Control::init();
//    WebInterface::init();
//
//    Display::printf("ready");
//
//    log_d("end");
//}
//
//void loop()
//{
//    const auto syncTimer{millis()};
//    Display::process(syncTimer);
//    Sensors::process(syncTimer);
//    Control::process(syncTimer);
//    WebInterface::process(syncTimer);
//    delay(1); // Necessário para o ESP TCP Async
//}

#include <Arduino.h>
#include <SPIFFS.h>

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    SPIFFS.begin(true);
}

void loop()
{
}