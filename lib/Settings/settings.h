#ifndef SETTINGS_H
#define SETTINGS_H

#include "../Logger/logger.h"
#include "appsettings.h"
#include <LittleFS.h>

class Settings {
  public:
    Settings(Logger &log, String filename = SETTINGS_FILENAME) : _logger(log), _filename(filename) {}
    // SimpleSettings(Logger &log, const char *filename = SETTINGS_FILENAME)
    //     : SimpleSettings(log, String(filename)) {}
    void begin();
    bool loadSettings(AppSettings &settings, std::function<void(AppSettings &)> getDefaultSettings);
    bool saveSettings(const AppSettings &newsettings, AppSettings &settings);
    static const char *SETTINGS_FILENAME;

  private:
    Logger &_logger; // Reference to the logger
    String _filename;
};

#endif // SETTINGS_H
