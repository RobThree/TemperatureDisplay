#include "appsettings.h"
#include "display.h"
#include "ota.h"
#include "sensor.h"
#include "settings.h"
#include "simplewifi.h"
#include "webserver.h"
#include <config.h>

// See include/example_config.h for configuration. Make sure you copy it
// to include/config.h and enter your configuration data there.

Logger logger(Serial);
OTA ota(logger);
SimpleWiFi wifi(logger);
Settings settings(logger);
Sensor sensor(logger);
Display display(logger, SCREEN_WIDTH, SCREEN_HEIGHT);
Webserver server(logger, LittleFS, HTTP_PORT);
SensorData lastreading;
AppSettings appsettings;

unsigned long previousMillis = 0; // Store the last time a measurement was taken

void handleReading();
void handleSettings();

void setup() {
    Serial.begin(SERIAL_BAUDRATE);

    display.begin();
    settings.begin();

    settings.loadSettings(appsettings, [](AppSettings &settings) {
        settings.updateInterval = UPDATEINTERVAL;
        settings.tempOffset = TEMPERATUREOFFSET;
        settings.humidityOffset = HUMIDITYOFFSET;
        settings.showFahrenheit = SHOWFAHRENHEIT ? true : false;
        strncpy(settings.deviceName, DEVICENAME, sizeof(settings.deviceName) - 1);
        settings.deviceName[sizeof(settings.deviceName) - 1] = '\0'; // Ensure null termination
    });

    wifi.begin(PORTALTIMEOUT, WIFICONNECTTIMEOUT, WIFICONNECTRETRIES, appsettings.deviceName);
    sensor.begin();

    server.serveStatic("/", "/index.html");
    server.serveStatic("/css", "/main.css");
    server.serveStatic("/js", "/app.js");
    server.serveStatic("/favicon.ico", "/favicon.svg");
    server.serveStatic("/settings", "/settings.html");
    server.on("/read", HTTP_GET, handleReading);
    server.on("/settings", HTTP_POST, handleSettings);
    server.useDefaultEndpoints();
    server.begin();

    ota.begin(appsettings.deviceName, OTAPASSWORD);

    display.setStatus(wifi.localIP());
    delay(2000);
}

void loop() {
    server.handleClient();
    ota.handle();
    wifi.ensureConnected();

    unsigned long currentMillis = millis(); // Get the current time

    // Check if it's time to take another measurement
    if (currentMillis - previousMillis >= appsettings.updateInterval) {
        previousMillis = currentMillis;

        SensorData reading = sensor.readData(appsettings.tempOffset, appsettings.humidityOffset);

        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Temperature: %s, humidity: %s",
                 reading.getTemperatureDisplay(appsettings.showFahrenheit).c_str(),
                 reading.getHumidityDisplay().c_str());
        logger.info(String(buffer));

        display.showMeasurements(reading, appsettings.showFahrenheit);

        lastreading = reading; // Store the last reading
    }
}

void handleReading() {
    JsonDocument response;
    JsonObject celsiusnode = response["celsius"].to<JsonObject>();
    celsiusnode["temperature"] = lastreading.getTemperatureDisplayValue(false);
    celsiusnode["offset"] = lastreading.offset_c;
    JsonObject fahrenheitnode = response["fahrenheit"].to<JsonObject>();
    fahrenheitnode["temperature"] = lastreading.getTemperatureDisplayValue(true);
    fahrenheitnode["offset"] = lastreading.offset_f;
    JsonObject humiditynode = response["humidity"].to<JsonObject>();
    humiditynode["relative_perc"] = lastreading.getHumidityDisplayValue();
    humiditynode["relative_perc_offset"] = lastreading.humidity_offset;
    JsonObject wifinode = response["wifi"].to<JsonObject>();
    wifinode["rssi"] = WiFi.RSSI();

    char lastUpdateStr[32];
    snprintf(lastUpdateStr, sizeof(lastUpdateStr), "%.2f seconds ago", (millis() - previousMillis) / 1000.0);
    response["lastupdate"] = lastUpdateStr;
    response["display"] = appsettings.showFahrenheit ? "f" : "c";
    response["devicename"] = appsettings.deviceName;
    response["updateinterval"] = appsettings.updateInterval;
    server.sendJson(response);
}

void handleSettings() {
    AppSettings newsettings;
    JsonDocument response;
    response["status"] = "";
    JsonArray errors = response["errors"].to<JsonArray>();
    newsettings.updateInterval = server.arg("updateinterval").toInt();
    if (newsettings.updateInterval < 1000 || newsettings.updateInterval > 60000) {
        errors.add("Update interval must be between 1000 and 60000 ms");
    }
    newsettings.showFahrenheit = server.arg("showfahrenheit") == "f";

    newsettings.tempOffset = server.arg("tempoffset").toFloat();
    if (newsettings.tempOffset < -50 || newsettings.tempOffset > 50) {
        errors.add("Temperature offset must be between -50 and 50");
    }
    newsettings.humidityOffset = server.arg("humidityoffset").toFloat();
    if (newsettings.humidityOffset < -50 || newsettings.humidityOffset > 50) {
        errors.add("Humidity offset must be between -50 and 50");
    }
    strncpy(newsettings.deviceName, server.arg("devicename").c_str(), sizeof(newsettings.deviceName) - 1);
    newsettings.deviceName[sizeof(newsettings.deviceName) - 1] = '\0'; // Ensure null termination
    if (strlen(newsettings.deviceName) == 0 || strlen(newsettings.deviceName) > 31) {
        errors.add("Device name must be between 1 and 31 characters");
    }

    if (errors.size() == 0 && settings.saveSettings(newsettings, appsettings)) {
        response["status"] = "success";
    } else {
        response["status"] = "error";
    }
    server.sendJson(response);
}