#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_SSD1306.h>
#include "../Logger/logger.h"

class Display {
public:
    Display(Logger &log, uint8_t width, uint8_t height);
    void begin();
    void setStatus(const String &status);
    void showMeasurements(float temperature, bool isFahrenheit, float humidity);
private:
    Logger &logger;  // Reference to the logger
    uint8_t width;
    uint8_t height;
    Adafruit_SSD1306 display;
};

#endif // DISPLAY_H
