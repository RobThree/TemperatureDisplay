#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <cmath>

struct SensorData {
    float temperature_c;
    float temperature_f;
    float offset_c;
    float offset_f;
    float humidity;
    float humidity_offset;

    static constexpr float factorCtoF = 9.0 / 5.0;
    static float toFahrenheit(float celsius) { return std::isnan(celsius) ? NAN : (celsius * factorCtoF) + 32.0; }
    static float offsetToFahrenheit(float celsius) { return celsius * factorCtoF; }

    SensorData(float temp_c = NAN, float offset_c = 0, float hum = NAN, float hum_offset = 0)
        : temperature_c(temp_c), temperature_f(toFahrenheit(temp_c)), offset_c(offset_c),
          offset_f(offsetToFahrenheit(offset_c)), humidity(hum), humidity_offset(hum_offset) {}

    float getTemperatureDisplayValue(bool showFahrenheit = false) const {
        return showFahrenheit ? temperature_f + offset_f : temperature_c + offset_c;
    }
    float getHumidityDisplayValue() const { return humidity + humidity_offset; }

    String getTemperatureDisplay(bool showFahrenheit = false) const {
        float t = getTemperatureDisplayValue(showFahrenheit);
        if (std::isnan(t)) {
            return "N/A";
        }

        char buffer[10];
        snprintf(buffer, sizeof(buffer), "%.2f %s", t, showFahrenheit ? "F" : "C");
        return String(buffer);
    }

    String getHumidityDisplay() const {
        float h = getHumidityDisplayValue();
        if (std::isnan(h)) {
            return "N/A";
        }

        char buffer[10];
        snprintf(buffer, sizeof(buffer), "%.2f %%RH", h);
        return String(buffer);
    }
};

#endif // SENSORDATA_H