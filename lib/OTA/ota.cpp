#include "ota.h"
#include <ArduinoOTA.h>

OTA::OTA(Logger &log) : _logger(log) {}

void OTA::begin(const char *const hostname, const char *const password) {
    ArduinoOTA.setPassword(hostname);
    ArduinoOTA.setHostname(password);
    ArduinoOTA.onStart([this]() {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        _logger.info(("Start updating " + type).c_str());
    });
    ArduinoOTA.onEnd([this]() { _logger.info("Update Complete"); });
    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
        _logger.info(("Progress: " + String((progress * 100) / total) + "%").c_str());
    });
    ArduinoOTA.onError([this](ota_error_t error) {
        _logger.error(("Error: " + String(error)).c_str());
        if (error == OTA_AUTH_ERROR) {
            _logger.error("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            _logger.error("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            _logger.error("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            _logger.error("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            _logger.error("End Failed");
        }
    });

    _logger.info("Starting OTA server");
    ArduinoOTA.begin();
}

void OTA::handle() { ArduinoOTA.handle(); }