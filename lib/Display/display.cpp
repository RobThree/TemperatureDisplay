#include "display.h"
#include "sensordata.h"
#include <Wire.h>

Display::Display(Logger &log, uint8_t w, uint8_t h) : _logger(log), _width(w), _height(h), _display(w, h, &Wire, -1) {}

void Display::begin() {
    Wire.begin();

    // Initialize SSD1306 display
    if (!_display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 0x3C is the I2C address of SSD1306
        _logger.error("SSD1306 allocation failed");
        ESP.restart();
    }

    _display.clearDisplay();
    _display.setRotation(2);
    _display.setTextSize(2);
    _display.setTextColor(SSD1306_WHITE);

    _display.display();
}

void Display::showMeasurements(SensorData &data, bool showFahrenheit) {
    _display.clearDisplay();

    _display.setCursor(0, 0);
    _display.print("Temp: ");
    _display.setCursor(15, 15);
    _display.print(data.getTemperatureDisplay(showFahrenheit));

    _display.setCursor(0, 30);
    _display.print("Humidity: ");
    _display.setCursor(15, 45);
    _display.print(data.getHumidityDisplay());

    _display.display();
}

void Display::setStatus(const String &status) {
    _display.clearDisplay();
    _display.setCursor(0, 0);
    _display.print(status);
    _display.display();
}