#include <Adafruit_SHT31.h>
#include <ArduinoJson.h>
#include "display.h"
#include "ota.h"
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include "simplewifi.h"
#include <config.h>

// See include/example_config.h for configuration. Make sure you copy it
// to include/config.h and enter your configuration data there.

Logger logger(Serial);
OTA ota(logger);
SimpleWiFi wifi(logger);
Display display(logger, SCREEN_WIDTH, SCREEN_HEIGHT);
Adafruit_SHT31 sht31 = Adafruit_SHT31();
ESP8266WebServer server(HTTP_PORT);
ESP8266HTTPUpdateServer httpUpdateServer;

float temperature;
float humidity;
unsigned long previousMillis = 0; // Store the last time a measurement was taken
const char *const SETTINGS_FILENAME = "/settings.bin";

struct Settings {
    unsigned long updateInterval;
    bool showFahrenheit;
    float tempOffset;
    float humidityOffset;
    char deviceName[32];
};

Settings settings;

void setStatus(Logger::Level level, const String &status) {
    logger.log(level, status);
    display.setStatus(status);
}

void restart(const String &status) {
    setStatus(Logger::WARN, status);
    delay(3000);
    ESP.restart();
}

bool loadSettings() {
    File file = LittleFS.open(SETTINGS_FILENAME, "r");
    if (!file || file.size() < sizeof(Settings)) {
        settings.updateInterval = UPDATEINTERVAL;
        settings.tempOffset = TEMPERATUREOFFSET;
        settings.humidityOffset = HUMIDITYOFFSET;
        settings.showFahrenheit = SHOWFAHRENHEIT ? true : false;
        strncpy(settings.deviceName, DEVICENAME, sizeof(settings.deviceName) - 1);
        settings.deviceName[sizeof(settings.deviceName) - 1] = '\0'; // Ensure null termination
        return false; // No settings file, use default settings
    }
    size_t bytesRead = file.read((uint8_t *)&settings, sizeof(Settings));
    file.close();
    return bytesRead == sizeof(Settings);
}

bool saveSettings(const Settings &newsettings) {
    File file = LittleFS.open(SETTINGS_FILENAME, "w");
    if (!file) {
        Serial.println("Failed to open file for writing");
        return false;
    }
    size_t bytesWritten = file.write((uint8_t *)&newsettings, sizeof(Settings));
    file.close();
    settings = newsettings;
    return bytesWritten == sizeof(Settings);
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

float factorCtoF = 9.0 / 5.0;
float toFahrenheit(float celsius) { return (celsius * factorCtoF) + 32.0; }
float offsetToFahrenheit(float celsius) { return celsius * factorCtoF; }

void setup() {
    Serial.begin(SERIAL_BAUDRATE);

    if (!LittleFS.begin()) {
        setStatus(Logger::ERROR, "Failed to mount filesystem");
    }

    if (loadSettings()) {
        setStatus(Logger::INFO, "Settings loaded");
    } else {
        setStatus(Logger::INFO, "Using default settings");
    }

    wifi.begin(PORTALTIMEOUT, WIFICONNECTTIMEOUT, WIFICONNECTRETRIES, settings.deviceName);

    // Initialize GXHT30
    if (!sht31.begin(0x44)) { // 0x44 is the I2C address of GXHT30
        setStatus(Logger::ERROR, "Couldn't find GXHT30 sensor!");
        ESP.restart();
    }

    httpUpdateServer.setup(&server);
    server.on("/read", HTTP_GET, []() {
        JsonDocument root;
        JsonObject celsiusnode = root["celsius"].to<JsonObject>();
        celsiusnode["temperature"] = temperature;
        celsiusnode["offset"] = settings.tempOffset;
        JsonObject fahrenheitnode = root["fahrenheit"].to<JsonObject>();
        fahrenheitnode["temperature"] = toFahrenheit(temperature);
        fahrenheitnode["offset"] = offsetToFahrenheit(settings.tempOffset);
        JsonObject humiditynode = root["humidity"].to<JsonObject>();
        humiditynode["relative_perc"] = humidity;
        humiditynode["relative_perc_offset"] = settings.humidityOffset;
        JsonObject wifinode = root["wifi"].to<JsonObject>();
        wifinode["rssi"] = WiFi.RSSI();

        char lastUpdateStr[32];
        snprintf(lastUpdateStr, sizeof(lastUpdateStr), "%.2f seconds ago",
                 (millis() - previousMillis) / 1000.0);
        root["lastupdate"] = lastUpdateStr;
        root["display"] = settings.showFahrenheit ? "f" : "c";
        root["devicename"] = settings.deviceName;
        root["updateinterval"] = settings.updateInterval;

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
    server.on("/favicon.ico", HTTP_GET, []() { serveFile("/favicon.svg", "image/svg+xml", 60 * 60 * 24); });
    server.on("/settings", HTTP_GET, []() { serveFile("/settings.html"); });
    server.on("/settings", HTTP_POST, []() {
        Settings newsettings;
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
                newsettings.deviceName[sizeof(newsettings.deviceName) - 1] = '\0'; // Ensure null termination
        if (strlen(newsettings.deviceName) == 0 || strlen(newsettings.deviceName) > 31) {
            errors.add("Device name must be between 1 and 31 characters");
        }

        if (errors.size() == 0 && saveSettings(newsettings)) {
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

    Serial.println("Starting HTTP server");
    server.begin();

    ota.begin(settings.deviceName, OTAPASSWORD);

    setStatus(Logger::INFO, wifi.localIP());
    delay(2000);
}

void loop() {
    server.handleClient();
    ota.handle();
    wifi.ensureConnected();

    unsigned long currentMillis = millis(); // Get the current time

    // Check if it's time to take another measurement
    if (currentMillis - previousMillis >= settings.updateInterval) {
        previousMillis = currentMillis;
        temperature = sht31.readTemperature() + settings.tempOffset;
        humidity = constrain(sht31.readHumidity() + settings.humidityOffset, 0, 100);

        Serial.printf("Temperature: %.2f C, %.2f F, humidity: %.2f %%RH\n", temperature,
                      toFahrenheit(temperature), humidity);

        display.showMeasurements(
            settings.showFahrenheit ? toFahrenheit(temperature) : temperature,
            settings.showFahrenheit,
            humidity
        );
    }
}