#include <Arduino.h>
#include <Wire.h>
#include <cstdio>
#include <esp_log.h>

#include "Display.hpp"
#include "Peripherals.hpp"
#include "Sensors.hpp"

#include <Adafruit_SSD1306.h>

namespace Display
{
    namespace
    {
        auto display{Adafruit_SSD1306{Peripherals::Display::WIDTH, Peripherals::Display::HEIGHT, &Peripherals::Display::I2C}};

        auto updateTimer{0UL};

        auto updateStatus(uint64_t syncTimer) -> void
        {
            if (syncTimer - updateTimer >= 2000UL)
            {
                updateTimer = syncTimer;

                //display.fillRect(0, 0, Peripherals::Display::WIDTH, 16, SSD1306_BLACK);
                //display.setCursor(0, 0);
                //display.printf("BAT %.0f %%", Sensors::battery());
                //display.display();
            }
        }
    } // namespace

    auto init() -> void
    {
        log_d("begin");

        log_d("initializing display");

        if (not display.begin(SSD1306_SWITCHCAPVCC, Peripherals::Display::ADDRESS))
        {
            log_e("failed to initialize display");
        }
        else
        {
            display.setRotation(0);
            display.setTextColor(SSD1306_WHITE);
            display.cp437(true);
            display.setTextSize(1);
            display.clearDisplay();
            display.print("Auto2");
            display.display();
        }

        log_d("end");
    }

    auto process(uint64_t syncTimer) -> void
    {
        Display::updateStatus(syncTimer);
    }

    auto printf(const char *format, ...) -> void
    {
        char buffer[256];

        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        display.fillRect(0, 16, Peripherals::Display::WIDTH, Peripherals::Display::HEIGHT - 16, SSD1306_BLACK);
        display.setCursor(0, 16);
        display.print(buffer);
        display.display();
    }
} // namespace Display