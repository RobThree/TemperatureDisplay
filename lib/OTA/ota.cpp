#include <ArduinoOTA.h>
#include "ota.h"

OTA::OTA(Logger &log)
    : logger(log) {}

void OTA::begin(const char *const hostname, const char *const password) {
    ArduinoOTA.setPassword(hostname);
    ArduinoOTA.setHostname(password);
    ArduinoOTA.onStart([this]() {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        logger.info(("Start updating " + type).c_str());
    });
    ArduinoOTA.onEnd([this]() {
        logger.info("Update Complete");
    });
    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
        logger.info(("Progress: " + String((progress * 100) / total) + "%").c_str());
    });
    ArduinoOTA.onError([this](ota_error_t error) {
        logger.error(("Error: " + String(error)).c_str());
        if (error == OTA_AUTH_ERROR) {
            logger.error("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            logger.error("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            logger.error("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            logger.error("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            logger.error("End Failed");
        }
    });

    logger.info("Starting OTA server");
    ArduinoOTA.begin();
}

void OTA::handle() {
    ArduinoOTA.handle();
}