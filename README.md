# Temperature Display

Uses an Wemos D1 mini with a GXHT30 I²C temperature & humidity sensor and a SSD1306 I²C 0.96" OLED display.

This project supports OTA updates, uses the WiFi manager to configure the WiFi and provides a `/read` endpoint to get the temperature and humidity as JSON and a `/reset` endpoint (`PUT`) to reset the module. The readings (and display) are updated every 2 seconds.

![Example](example.jpg)