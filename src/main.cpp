#include <Adafruit_GFX.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Wire.h>
#include <config.h>

// See include/example_config.h for configuration. Make sure you copy it
// to include/config.h and enter your configuration data there.

Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // -1 means no reset pin
ESP8266WebServer server(HTTP_PORT);
ESP8266HTTPUpdateServer httpUpdateServer;

float temp;
float humidity;

void setStatus(const String &status) {
    Serial.println(status);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(status);
    display.display();
}

void restart(const String &status) {
    setStatus(status);
    delay(3000);
    ESP.restart();
}

void setup() {
    Serial.begin(SERIAL_BAUDRATE);
    Wire.begin();

    WiFiManager wifiManager;
    wifiManager.setConfigPortalTimeout(PORTALTIMEOUT);
    wifiManager.setConnectTimeout(WIFICONNECTTIMEOUT);
    wifiManager.setConnectRetries(WIFICONNECTRETRIES);
    wifiManager.setWiFiAutoReconnect(true);
    if (!wifiManager.autoConnect(DEVICENAME)) {
        restart("Autconnect failed...");
    }

    // Initialize GXHT30
    if (!sht31.begin(0x44)) { // 0x44 is the I2C address of GXHT30
        setStatus("Couldn't find GXHT30 sensor!");
        ESP.restart();
    }

    // Initialize SSD1306 display
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 0x3C is the I2C address of SSD1306
        setStatus("SSD1306 allocation failed");
        ESP.restart();
    }

    display.clearDisplay();
    display.setRotation(2);
    display.setTextSize(2); // Text size
    display.setTextColor(SSD1306_WHITE);

    httpUpdateServer.setup(&server);
    server.on("/read", HTTP_GET, []() {
        JsonDocument root;
        root["temperature_c"] = temp;
        root["temperature_f"] = temp * 9 / 5 + 32;
        root["offset"] = TEMPERATUREOFFSET;
        root["humidity"] = humidity;
        root["rssi"] = WiFi.RSSI();

        String response;
        serializeJson(root, response); // Convert JSON object to string

        server.send(200, "application/json", response);
    });

    server.on("/reset", HTTP_PUT, []() {
        server.send(200, "text/html", "reset");
        restart("Restarting");
    });

    server.onNotFound([]() { server.send(404, "text/plain", "File not found"); });

    Serial.println("Starting HTTP server");
    server.begin();

    ArduinoOTA.setPassword(OTAPASSWORD);
    ArduinoOTA.setHostname(DEVICENAME); // Set the OTA device hostname
    ArduinoOTA.onStart([]() {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        setStatus("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() { setStatus("Update Complete"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        setStatus("Progress: " + String((progress * 100) / total) + "%");
    });
    ArduinoOTA.onError([](ota_error_t error) {
        setStatus("Error: " + String(error));
        if (error == OTA_AUTH_ERROR)
            setStatus("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
            setStatus("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
            setStatus("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
            setStatus("Receive Failed");
        else if (error == OTA_END_ERROR)
            setStatus("End Failed");
    });

    setStatus("Starting OTA server");
    ArduinoOTA.begin();

    setStatus(WiFi.localIP().toString());
    delay(2000);
}

unsigned long previousMillis = 0; // Store the last time the display was updated

void loop() {
    server.handleClient();
    ArduinoOTA.handle();
    if ((WiFi.status() != WL_CONNECTED)) {
        setStatus("WiFi not connected, reconnecting...");
        WiFi.reconnect();
        unsigned long start = millis();

        while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
            delay(100);
        }

        if (WiFi.status() == WL_CONNECTED) {
            setStatus("WiFi reconnected");
        } else {
            setStatus("Failed to reconnect to WiFi. Retrying...");
            delay(1000);
        }
    }

    unsigned long currentMillis = millis(); // Get the current time

    // Check if it's time to update the display
    if (currentMillis - previousMillis >= UPDATEINTERVAL) {
        previousMillis = currentMillis;
        temp = sht31.readTemperature() + TEMPERATUREOFFSET;
        humidity = sht31.readHumidity();

        Serial.printf("Temperature: %.2f C, Humidity: %.2f %%\n", temp, humidity);

        // Clear display
        display.clearDisplay();

        // Print temperature
        display.setCursor(0, 0);
        display.print("Temp: ");
        display.setCursor(15, 15);
        display.print(temp);
        display.print(" C");

        // Print humidity
        display.setCursor(0, 30);
        display.print("Humidity: ");
        display.setCursor(15, 45);
        display.print(humidity);
        display.print(" %");

        // Update display
        display.display();
    }
}
