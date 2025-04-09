#include "simplewifi.h"

SimpleWiFi::SimpleWiFi(Logger &log) : logger(log) {}

void SimpleWiFi::begin(unsigned long portalTimeout, unsigned long wifiConnectTimeout, unsigned long wifiConnectRetries,
                       const char *apName, const char *apPassword) {
    logger.info("Starting WiFi");
    wifiManager.setConfigPortalTimeout(portalTimeout);
    wifiManager.setConnectTimeout(wifiConnectTimeout);
    wifiManager.setConnectRetries(wifiConnectRetries);
    wifiManager.setWiFiAutoReconnect(true);
    if (!wifiManager.autoConnect(apName, apPassword)) {
        logger.error("Autoconnect failed. Restarting...");
        delay(3000);
        ESP.restart();
    }
}

void SimpleWiFi::ensureConnected() {
    if ((WiFi.status() != WL_CONNECTED)) {
        logger.warn("WiFi not connected, reconnecting...");
        WiFi.reconnect();
        unsigned long start = millis();

        while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
            delay(100);
        }

        if (WiFi.status() == WL_CONNECTED) {
            logger.error("WiFi reconnected");
        } else {
            logger.warn("Failed to reconnect to WiFi. Retrying...");
            delay(1000);
        }
    }
}

String SimpleWiFi::localIP() { return WiFi.localIP().toString(); }