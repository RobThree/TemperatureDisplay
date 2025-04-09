#include "sensor.h"

Sensor::Sensor(Logger &log) : logger(log) {}

void Sensor::begin(uint8_t i2cAddress) {
    if (!sht31.begin(i2cAddress)) {
        logger.error("Couldn't find GXHT30 sensor!");
        ESP.restart();
    }
}

SensorData Sensor::readData(float tempOffset, float humidityOffset) {
    SensorData data(sht31.readTemperature(), tempOffset, sht31.readHumidity(), humidityOffset);
    if (isnan(data.temperature_c) || isnan(data.humidity)) {
        logger.error("Failed to read sensor data");
    }
    return data;
}