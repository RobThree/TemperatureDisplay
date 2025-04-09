#include "settings.h"
#include "appsettings.h"

const char *Settings::SETTINGS_FILENAME = "/settings.bin";

void Settings::begin() { LittleFS.begin(); }

bool Settings::loadSettings(AppSettings &settings, std::function<void(AppSettings &)> getDefaultSettings) {
    File file = LittleFS.open(_filename, "r");
    if (!file || file.size() < sizeof(AppSettings)) {
        getDefaultSettings(settings);
        _logger.warn("Using default settings");
        return true; // No settings file, used default settings
    }
    size_t bytesRead = file.read((uint8_t *)&settings, sizeof(AppSettings));
    file.close();
    if (bytesRead == sizeof(AppSettings)) {
        _logger.info("Settings loaded successfully");
        return true;
    } else {
        _logger.error("Failed to read settings");
    }
    return false; // Failed to read settings
}

bool Settings::saveSettings(const AppSettings &newsettings, AppSettings &settings) {
    File file = LittleFS.open(_filename, "w");
    if (!file) {
        _logger.error("Failed to open file for writing");
        return false;
    }
    size_t bytesWritten = file.write((uint8_t *)&newsettings, sizeof(AppSettings));
    file.close();
    settings = newsettings;
    if (bytesWritten == sizeof(AppSettings)) {
        _logger.info("Settings saved successfully");
        return true;
    } else {
        _logger.error("Failed to write settings");
    }
    return false; // Failed to write settings
}