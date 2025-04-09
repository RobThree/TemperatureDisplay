#include "appsettings.h"
#include "display.h"
#include "ota.h"
#include "sensor.h"
#include "settings.h"
#include "simplewifi.h"
#include <ArduinoJson.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <config.h>

// See include/example_config.h for configuration. Make sure you copy it
// to include/config.h and enter your configuration data there.

Logger logger(Serial);
OTA ota(logger);
SimpleWiFi wifi(logger);
Settings settings(logger);
Sensor sht31(logger);
Display display(logger, SCREEN_WIDTH, SCREEN_HEIGHT);
ESP8266WebServer server(HTTP_PORT);
ESP8266HTTPUpdateServer httpUpdateServer;

SensorData lastreading;
AppSettings appsettings;

unsigned long previousMillis = 0; // Store the last time a measurement was taken

void setStatus(Logger::Level level, const String &status) {
    logger.log(level, status);
    display.setStatus(status);
}

void restart(const String &status) {
    setStatus(Logger::WARN, status);
    delay(3000);
    ESP.restart();
}

void serveFile(const char *path, const char *contentType = "text/html", int cacheTTL = 300) {
    File file = LittleFS.open(path, "r");
    if (!file) {
        server.send(404, "text/plain", "File not found");
        return;
    }
    server.sendHeader("Cache-Control", "max-age=" + String(cacheTTL));
    server.streamFile(file, contentType);
    file.close();
}

void setup() {
    Serial.begin(SERIAL_BAUDRATE);

    settings.begin();

    settings.loadSettings(
        appsettings, [](AppSettings &settings) {
        settings.updateInterval = UPDATEINTERVAL;
        settings.tempOffset = TEMPERATUREOFFSET;
        settings.humidityOffset = HUMIDITYOFFSET;
        settings.showFahrenheit = SHOWFAHRENHEIT ? true : false;
        strncpy(settings.deviceName, DEVICENAME, sizeof(settings.deviceName) - 1);
        settings.deviceName[sizeof(settings.deviceName) - 1] = '\0'; // Ensure null termination
    });

    wifi.begin(PORTALTIMEOUT, WIFICONNECTTIMEOUT, WIFICONNECTRETRIES, appsettings.deviceName);

    sht31.begin(); // Initialize SHT31 sensor

    httpUpdateServer.setup(&server);
    server.on("/read", HTTP_GET, []() {
        JsonDocument root;
        JsonObject celsiusnode = root["celsius"].to<JsonObject>();
        celsiusnode["temperature"] = lastreading.getTemperatureDisplayValue(false);
        celsiusnode["offset"] = lastreading.offset_c;
        JsonObject fahrenheitnode = root["fahrenheit"].to<JsonObject>();
        fahrenheitnode["temperature"] = lastreading.getTemperatureDisplayValue(true);
        fahrenheitnode["offset"] = lastreading.offset_f;
        JsonObject humiditynode = root["humidity"].to<JsonObject>();
        humiditynode["relative_perc"] = lastreading.getHumidityDisplayValue();
        humiditynode["relative_perc_offset"] = lastreading.humidity_offset;
        JsonObject wifinode = root["wifi"].to<JsonObject>();
        wifinode["rssi"] = WiFi.RSSI();

        char lastUpdateStr[32];
        snprintf(lastUpdateStr, sizeof(lastUpdateStr), "%.2f seconds ago",
                 (millis() - previousMillis) / 1000.0);
        root["lastupdate"] = lastUpdateStr;
        root["display"] = appsettings.showFahrenheit ? "f" : "c";
        root["devicename"] = appsettings.deviceName;
        root["updateinterval"] = appsettings.updateInterval;

        String response;
        serializeJson(root, response);

        server.send(200, "application/json", response);
    });

    server.on("/reset", HTTP_PUT, []() {
        server.send(200, "text/html", "reset");
        restart("Restarting");
    });

    server.on("/", HTTP_GET, []() { serveFile("/index.html"); });
    server.on("/css", HTTP_GET, []() { serveFile("/main.css", "text/css"); });
    server.on("/js", HTTP_GET, []() { serveFile("/app.js", "text/javascript"); });
    server.on("/favicon.ico", HTTP_GET,
              []() { serveFile("/favicon.svg", "image/svg+xml", 60 * 60 * 24); });
    server.on("/settings", HTTP_GET, []() { serveFile("/settings.html"); });
    server.on("/settings", HTTP_POST, []() {
        AppSettings newsettings;
        JsonDocument root;
        root["status"] = "";
        JsonArray errors = root["errors"].to<JsonArray>();
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
        strncpy(newsettings.deviceName, server.arg("devicename").c_str(),
                sizeof(newsettings.deviceName) - 1);
        newsettings.deviceName[sizeof(newsettings.deviceName) - 1] =
            '\0'; // Ensure null termination
        if (strlen(newsettings.deviceName) == 0 || strlen(newsettings.deviceName) > 31) {
            errors.add("Device name must be between 1 and 31 characters");
        }

        if (errors.size() == 0 && settings.saveSettings(newsettings, appsettings)) {
            root["status"] = "success";
            setStatus(Logger::INFO, "Settings saved");
        } else {
            root["status"] = "error";
        }
        String response;
        serializeJson(root, response);

        server.send(200, "application/json", response);
    });

    server.onNotFound([]() { server.send(404, "text/plain", "File not found"); });

    logger.info("Starting HTTP server");
    server.begin();

    ota.begin(appsettings.deviceName, OTAPASSWORD);

    setStatus(Logger::INFO, wifi.localIP());
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

        SensorData reading = sht31.readData(appsettings.tempOffset, appsettings.humidityOffset);

        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Temperature: %s, humidity: %s",
                 reading.getTemperatureDisplay(appsettings.showFahrenheit).c_str(),
                 reading.getHumidityDisplay().c_str());
        logger.info(String(buffer));

        display.showMeasurements(reading, appsettings.showFahrenheit);

        lastreading = reading; // Store the last reading
    }
}