#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "../Logger/logger.h"
#include <ArduinoJson.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>

class Webserver {
  public:
    Webserver(Logger &log, FS fs, int port = 80) : logger(log), fs(fs), server(port) {
        httpUpdateServer.setup(&server);
        server.onNotFound([this]() { server.send(404, "text/plain", "File not found"); });
    };
    void useDefaultEndpoints();
    void begin();
    void handleClient();
    void sendJson(const JsonDocument &json, int httpCode = 200);
    const String &arg(const String &name) const;
    void serveStatic(const char *, const char *path, const char *cacheheader = "max-age=300");
    void on(const String &uri, ESP8266WebServer::THandlerFunction fn) { server.on(uri, fn); }
    void on(const String &uri, HTTPMethod method, ESP8266WebServer::THandlerFunction fn) { server.on(uri, method, fn); }
    void on(const String &uri, HTTPMethod method, ESP8266WebServer::THandlerFunction fn,
            ESP8266WebServer::THandlerFunction ufn) {
        server.on(uri, method, fn, ufn);
    }

  private:
    Logger &logger; // Reference to the logger
    FS fs;
    ESP8266WebServer server;
    ESP8266HTTPUpdateServer httpUpdateServer;
};

#endif // WEBSERVER_H
