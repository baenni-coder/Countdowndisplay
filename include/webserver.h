#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>
#include <WiFi.h>

// HTTP-Methoden Definitionen (fehlend in ESPAsyncWebServer 3.6.0)
#ifndef HTTP_ANY
typedef enum {
    HTTP_GET     = 0b00000001,
    HTTP_POST    = 0b00000010,
    HTTP_DELETE  = 0b00000100,
    HTTP_PUT     = 0b00001000,
    HTTP_PATCH   = 0b00010000,
    HTTP_HEAD    = 0b00100000,
    HTTP_OPTIONS = 0b01000000,
    HTTP_ANY     = 0b01111111,
} WebRequestMethod;
#endif

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
