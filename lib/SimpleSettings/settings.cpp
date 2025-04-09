#include "appsettings.h"
#include "settings.h"

const char *Settings::SETTINGS_FILENAME = "/settings.bin";

void Settings::begin() {
    LittleFS.begin();
}

bool Settings::loadSettings(AppSettings &settings, std::function<void(AppSettings&)> getDefaultSettings) {
    File file = LittleFS.open(filename, "r");
    if (!file || file.size() < sizeof(AppSettings)) {
        getDefaultSettings(settings);
        logger.warn("Using default settings");
        return true; // No settings file, used default settings
    }
    size_t bytesRead = file.read((uint8_t *)&settings, sizeof(AppSettings));
    file.close();
    if (bytesRead == sizeof(AppSettings)) {
        logger.info("Settings loaded successfully");
        return true;
    } else {
        logger.error("Failed to read settings");
    }
    return false; // Failed to read settings
}

bool Settings::saveSettings(const AppSettings &newsettings, AppSettings &settings) {
    File file = LittleFS.open(filename, "w");
    if (!file) {
        logger.error("Failed to open file for writing");
        return false;
    }
    size_t bytesWritten = file.write((uint8_t *)&newsettings, sizeof(AppSettings));
    file.close();
    settings = newsettings;
    return bytesWritten == sizeof(AppSettings);
}