#ifndef SENSOR_H
#define SENSOR_H

#include "../Logger/logger.h"
#include "sensordata.h"
#include <Adafruit_SHT31.h>

class Sensor {
  public:
    Sensor(Logger &log);
    void begin(uint8_t i2cAddress = 0x44); // Default I2C address for SHT31
    SensorData readData(float tempOffset, float humidityOffset);

  private:
    Logger &logger; // Reference to the logger
    Adafruit_SHT31 sht31 = Adafruit_SHT31();
};

#endif // SENSOR_H