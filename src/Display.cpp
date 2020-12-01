#include <Arduino.h>
#include <Wire.h>
#include <cstdio>
#include <esp_log.h>

#include "Display.hpp"
#include "Peripherals.hpp"

#include <Adafruit_SSD1306.h>

namespace Display
{
    static auto display{Adafruit_SSD1306{Peripherals::Display::WIDTH, Peripherals::Display::HEIGHT, &Peripherals::Display::I2C}};
    static auto updateTimer{0UL};
    static auto needUpdate{false};

    auto init() -> void
    {
        log_d("begin");

        if (not display.begin(SSD1306_SWITCHCAPVCC, Peripherals::Display::ADDRESS))
        {
            log_e("failed to initialize display");
        }
        else
        {
            display.setRotation(1);
            display.setTextColor(SSD1306_WHITE);
            display.cp437(true);

            display.clearDisplay();
            display.setTextSize(2);
            display.setCursor(0, 0);
            display.print("Auto2");
            display.display();

            display.setTextSize(1);
            display.clearDisplay();

            log_d("successfully initialized display");
        }

        log_d("end");
    }

    auto process() -> void
    {
        if (millis() - updateTimer >= 200UL)
        {
            updateTimer = millis();

            if (needUpdate)
            {
                display.display();
                display.clearDisplay();
                needUpdate = false;
            }
        }
    }

    auto printf(int16_t x, int16_t y, const char *format, ...) -> void
    {
        char *buffer;

        va_list args;
        va_start(args, format);
        vasprintf(&buffer, format, args);
        va_end(args);

        display.setCursor(x, y);
        display.print(buffer);
        needUpdate = true;

        free(buffer);
    }
} // namespace Display