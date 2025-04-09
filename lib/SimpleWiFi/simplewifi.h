#ifndef SIMPLEWIFI_H
#define SIMPLEWIFI_H

#include "../Logger/logger.h"
#include "../statuscallback.h"
#include <WiFiManager.h>

class SimpleWiFi {
  public:
    SimpleWiFi(Logger &log) : _logger(log), _statuscallback(emptyStatus) {}

    void begin(unsigned long portalTimeout, unsigned long wifiConnectTimeout, unsigned long wifiConnectRetries,
               const char *apName, const char *apPassword = (const char *)__null);
    void ensureConnected();
    String localIP();
    void onStatus(StatusCallback callback) { _statuscallback = callback; }

  private:
    Logger &_logger; // Reference to the logger
    WiFiManager _wifiManager;
    StatusCallback _statuscallback;
};

#endif // SIMPLEWIFI_H