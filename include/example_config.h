// COPY THIS FILE AND RENAME IT TO "config.h" AND ENTER ALL YOUR CONFIGURATION DATA BELOW

#ifndef CONFIG_H
#define CONFIG_H

// These can all be set in the web interface and are used as defaults
#define UPDATEINTERVAL      2000    // Update interval in milliseconds
#define SHOWFAHRENHEIT      false   // Show Fahrenheit on display (celcius otherwise)
#define TEMPERATUREOFFSET   0.0     // Temperature offset / correction in Celcius
#define HUMIDITYOFFSET      0.0     // Humidity offset / correction in percentage
#define DEVICENAME          "devicename-here"

// These can only be set by re-flashing the firmware
#define OTAPASSWORD         "my_ota_secret"
#define PORTALTIMEOUT       90
#define WIFICONNECTTIMEOUT  10
#define WIFICONNECTRETRIES  255
#define HTTP_PORT           80
#define SERIAL_BAUDRATE     115200

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#endif