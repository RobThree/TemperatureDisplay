#include "display.h"
#include <Wire.h>

Display::Display(Logger &log, uint8_t w, uint8_t h)
    : logger(log), width(w), height(h), display(w, h, &Wire, -1) {}  

void Display::begin() {
    Wire.begin();

    // Initialize SSD1306 display
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 0x3C is the I2C address of SSD1306
        logger.error("SSD1306 allocation failed");
        ESP.restart();
    }

    display.clearDisplay();
    display.setRotation(2);
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
}

void Display::showMeasurements(float temperature, bool isFahrenheit, float humidity) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Temp: ");
    display.print(temperature);
    display.print(isFahrenheit ? " F" : " C");
    display.setCursor(0, 16);
    display.print("Humidity: ");
    display.print(humidity);
    display.print(" %");
    display.display();
}

void Display::setStatus(const String &status) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(status);
    display.display();
}