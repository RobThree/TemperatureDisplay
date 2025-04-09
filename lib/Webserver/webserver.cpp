#include "webserver.h"

void Webserver::useDefaultEndpoints() {
    _server.on("/reset", HTTP_PUT, [this]() {
        _server.send(200, "text/html", "reset");
        _logger.warn("Restarting");
        ESP.restart();
    });
}

void Webserver::serveStatic(const char *uri, const char *path, const char *cacheheader) {
    _server.serveStatic(uri, _fs, path, cacheheader);
}

void Webserver::begin() {
    _fs.begin();
    _server.begin();
    _logger.info("HTTP server started");
}

void Webserver::handleClient() { _server.handleClient(); }

void Webserver::sendJson(const JsonDocument &json, int httpCode) {
    String response;
    serializeJson(json, response);
    _server.send(httpCode, "application/json", response);
}

const String &Webserver::arg(const String &name) const { return _server.arg(name); }