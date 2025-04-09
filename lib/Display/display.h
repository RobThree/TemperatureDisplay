#ifndef DISPLAY_H
#define DISPLAY_H

#include "../Logger/logger.h"
#include "sensordata.h"
#include <Adafruit_SSD1306.h>

class Display {
  public:
    Display(Logger &log, uint8_t width, uint8_t height);
    void begin();
    void setStatus(const String &status);
    void showMeasurements(SensorData &data, bool showFahrenheit);

  private:
    Logger &_logger; // Reference to the logger
    uint8_t _width;
    uint8_t _height;
    Adafruit_SSD1306 _display;
};

#endif // DISPLAY_H
