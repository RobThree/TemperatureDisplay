#ifndef SETTINGS_H
#define SETTINGS_H

#include "../Logger/logger.h"
#include "../statuscallback.h"
#include "appsettings.h"
#include <LittleFS.h>

class Settings {
  public:
    Settings(Logger &log, String filename = SETTINGS_FILENAME) : _logger(log), _filename(filename), _statuscallback(emptyStatus) {}
    // SimpleSettings(Logger &log, const char *filename = SETTINGS_FILENAME)
    //     : SimpleSettings(log, String(filename)) {}
    void begin();
    bool loadSettings(AppSettings &settings, std::function<void(AppSettings &)> getDefaultSettings);
    bool saveSettings(const AppSettings &newsettings, AppSettings &settings);
    void onStatus(StatusCallback callback) { _statuscallback = callback; }
    static const char *SETTINGS_FILENAME;

  private:
    Logger &_logger; // Reference to the logger
    String _filename;
    StatusCallback _statuscallback;
};

#endif // SETTINGS_H
