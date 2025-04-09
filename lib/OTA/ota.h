#ifndef OTA_H
#define OTA_H

#include "../Logger/logger.h"
#include "../statuscallback.h"
#include <ArduinoOTA.h>

class OTA {
  public:
    OTA(Logger &log) : _logger(log), _statuscallback(emptyStatus) {};
    void begin(const char *const hostname, const char *const password);
    void handle();
    void onStatus(StatusCallback callback) { _statuscallback = callback; }

  private:
    Logger &_logger; // Reference to the logger
    StatusCallback _statuscallback;
};

#endif // OTA_H
