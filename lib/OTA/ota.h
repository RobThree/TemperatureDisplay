#ifndef OTA_H
#define OTA_H

#include <ArduinoOTA.h>
#include "../Logger/logger.h"

class OTA {
public:
    OTA(Logger &log);
    void begin(const char *const hostname, const char *const password);
    void handle();
private:
    Logger &logger;  // Reference to the logger
};

#endif // OTA_H
