#include "webserver.h"

void Webserver::useDefaultEndpoints() {
    server.on("/reset", HTTP_PUT, [this]() {
        server.send(200, "text/html", "reset");
        logger.warn("Restarting");
        ESP.restart();
    });
}

void Webserver::serveStatic(const char *uri, const char *path, const char *cacheheader) {
    server.serveStatic(uri, fs, path, cacheheader);
}

void Webserver::begin() {
    fs.begin();
    server.begin();
    logger.info("HTTP server started");
}

void Webserver::handleClient() { server.handleClient(); }

void Webserver::sendJson(const JsonDocument &json, int httpCode) {
    String response;
    serializeJson(json, response);
    server.send(httpCode, "application/json", response);
}

const String &Webserver::arg(const String &name) const { return server.arg(name); }