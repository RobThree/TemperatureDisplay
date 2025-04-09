#include "sensor.h"

Sensor::Sensor(Logger &log) : _logger(log) {}

void Sensor::begin(uint8_t i2cAddress) {
    if (!_sht31.begin(i2cAddress)) {
        _logger.error("Couldn't find GXHT30 sensor!");
        ESP.restart();
    }
}

SensorData Sensor::readData(float tempOffset, float humidityOffset) {
    SensorData data(_sht31.readTemperature(), tempOffset, _sht31.readHumidity(), humidityOffset);
    if (isnan(data.temperature_c) || isnan(data.humidity)) {
        _logger.error("Failed to read sensor data");
    }
    return data;
}