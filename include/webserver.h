#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

class WebServerManager {
public:
    WebServerManager();
    bool begin();
    void handle();

    bool startAP();
    bool connectToWiFi(const String& ssid, const String& password);
    String getIPAddress();

private:
    AsyncWebServer server;
    bool apMode;

    void setupRoutes();
    void handleGetCountdowns(AsyncWebServerRequest* request);
    void handleAddCountdown(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total);
    void handleUpdateCountdown(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total);
    void handleDeleteCountdown(AsyncWebServerRequest* request);
    void handleGetWiFi(AsyncWebServerRequest* request);
    void handleSetWiFi(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total);
    void handleScanCard(AsyncWebServerRequest* request);
};

extern WebServerManager webServer;

#endif
