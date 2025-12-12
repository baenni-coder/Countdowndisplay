#include "webserver.h"
#include "storage.h"
#include "rfid.h"
#include "config.h"

WebServerManager webServer;

WebServerManager::WebServerManager() : server(80), apMode(true) {
}

bool WebServerManager::begin() {
    // Versuche gespeicherte WiFi Credentials zu laden
    String ssid, password;
    if (storage.getWiFiCredentials(ssid, password) && !ssid.isEmpty()) {
        if (connectToWiFi(ssid, password)) {
            apMode = false;
        } else {
            Serial.println("Verbindung zu gespeichertem WiFi fehlgeschlagen, starte AP");
            startAP();
        }
    } else {
        startAP();
    }

    setupRoutes();
    server.begin();

    Serial.print("Webserver gestartet auf: ");
    Serial.println(getIPAddress());

    return true;
}

bool WebServerManager::startAP() {
    WiFi.mode(WIFI_AP);
    bool success = WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);

    if (success) {
        Serial.println("Access Point gestartet");
        Serial.print("SSID: ");
        Serial.println(WIFI_SSID);
        Serial.print("Password: ");
        Serial.println(WIFI_PASSWORD);
        Serial.print("IP: ");
        Serial.println(WiFi.softAPIP());
        apMode = true;
    }

    return success;
}

bool WebServerManager::connectToWiFi(const String& ssid, const String& password) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    Serial.print("Verbinde zu WiFi: ");
    Serial.println(ssid);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi verbunden!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("WiFi Verbindung fehlgeschlagen");
        return false;
    }
}

String WebServerManager::getIPAddress() {
    if (apMode) {
        return WiFi.softAPIP().toString();
    } else {
        return WiFi.localIP().toString();
    }
}

void WebServerManager::handle() {
    // AsyncWebServer läuft asynchron, nichts zu tun
}

void WebServerManager::setupRoutes() {
    // Serve static files from LittleFS
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    // API Endpoints

    // GET /api/countdowns - Alle Countdowns abrufen
    server.on("/api/countdowns", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleGetCountdowns(request);
    });

    // POST /api/countdowns - Neuen Countdown hinzufügen
    server.on("/api/countdowns", HTTP_POST, [](AsyncWebServerRequest* request) {}, NULL,
        [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, data, len);

            if (error) {
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON\"}");
                return;
            }

            Countdown cd;
            cd.uid = doc["uid"].as<String>();
            cd.name = doc["name"].as<String>();
            cd.targetDate = doc["targetDate"].as<String>();
            cd.imagePath = doc["imagePath"] | "";
            cd.active = doc["active"].as<bool>();

            if (storage.addCountdown(cd)) {
                request->send(200, "application/json", "{\"success\":true}");
            } else {
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Konnte Countdown nicht hinzufügen\"}");
            }
        });

    // PUT /api/countdowns/:uid - Countdown aktualisieren
    server.on("^\\/api\\/countdowns\\/(.+)$", HTTP_PUT, [](AsyncWebServerRequest* request) {}, NULL,
        [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            Serial.println("PUT Request empfangen");
            Serial.print("URL: ");
            Serial.println(request->url());

            String uid = request->url().substring(request->url().lastIndexOf('/') + 1);
            Serial.print("Extrahierte UID: ");
            Serial.println(uid);

            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, data, len);

            if (error) {
                Serial.print("JSON Parse Fehler: ");
                Serial.println(error.c_str());
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON\"}");
                return;
            }

            Countdown cd;
            cd.uid = doc["uid"].as<String>();
            cd.name = doc["name"].as<String>();
            cd.targetDate = doc["targetDate"].as<String>();
            cd.imagePath = doc["imagePath"] | "";
            cd.active = doc["active"].as<bool>();

            Serial.print("Update Countdown: ");
            Serial.print(cd.name);
            Serial.print(", ImagePath: ");
            Serial.println(cd.imagePath);

            bool result = storage.updateCountdown(uid, cd);
            Serial.print("Update Result: ");
            Serial.println(result ? "Erfolgreich" : "Fehlgeschlagen");

            if (result) {
                request->send(200, "application/json", "{\"success\":true}");
            } else {
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Konnte Countdown nicht aktualisieren\"}");
            }
        });

    // DELETE /api/countdowns/:uid - Countdown löschen
    server.on("^\\/api\\/countdowns\\/(.+)$", HTTP_DELETE, [this](AsyncWebServerRequest* request) {
        handleDeleteCountdown(request);
    });

    // GET /api/wifi - WiFi Einstellungen abrufen
    server.on("/api/wifi", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleGetWiFi(request);
    });

    // POST /api/wifi - WiFi Einstellungen setzen
    server.on("/api/wifi", HTTP_POST, [](AsyncWebServerRequest* request) {}, NULL,
        [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, data, len);

            if (error) {
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON\"}");
                return;
            }

            String ssid = doc["ssid"].as<String>();
            String password = doc["password"].as<String>();

            if (storage.saveWiFiCredentials(ssid, password)) {
                request->send(200, "application/json", "{\"success\":true,\"message\":\"WiFi Einstellungen gespeichert. Neustart erforderlich.\"}");
            } else {
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Konnte WiFi Einstellungen nicht speichern\"}");
            }
        });

    // GET /api/scan-card - Scanne RFID Karte
    server.on("/api/scan-card", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleScanCard(request);
    });

    // GET /api/status - System Status
    server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        DynamicJsonDocument doc(512);
        doc["apMode"] = apMode;
        doc["ip"] = getIPAddress();
        doc["ssid"] = apMode ? WIFI_SSID : WiFi.SSID();

        String output;
        serializeJson(doc, output);
        request->send(200, "application/json", output);
    });

    // Restart ESP
    server.on("/api/restart", HTTP_POST, [](AsyncWebServerRequest* request) {
        request->send(200, "application/json", "{\"success\":true,\"message\":\"Neustarte...\"}");
        delay(500);
        ESP.restart();
    });

    // POST /api/upload-image - Bild hochladen
    server.on("/api/upload-image", HTTP_POST,
        [](AsyncWebServerRequest* request) {
            // Wird aufgerufen, wenn der Upload abgeschlossen ist
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Bild erfolgreich hochgeladen\"}");
        },
        [](AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data, size_t len, bool final) {
            // Multipart File Upload Handler
            static File uploadFile;

            if (index == 0) {
                // Start des Uploads - erstelle Datei
                Serial.println("Starte Bild-Upload: " + filename);

                // Erstelle /images Verzeichnis falls nicht vorhanden
                if (!LittleFS.exists("/images")) {
                    LittleFS.mkdir("/images");
                }

                // Öffne Datei zum Schreiben
                String path = "/images/" + filename;
                uploadFile = LittleFS.open(path, "w");
                if (!uploadFile) {
                    Serial.println("Fehler beim Erstellen der Datei");
                    request->send(500, "application/json", "{\"success\":false,\"error\":\"Konnte Datei nicht erstellen\"}");
                    return;
                }
            }

            // Schreibe Daten
            if (uploadFile && len) {
                uploadFile.write(data, len);
            }

            if (final) {
                // Upload abgeschlossen
                if (uploadFile) {
                    uploadFile.close();
                }
                Serial.println("Bild-Upload abgeschlossen: " + filename);
            }
        }
    );

    // GET /api/images - Liste aller Bilder
    server.on("/api/images", HTTP_GET, [](AsyncWebServerRequest* request) {
        DynamicJsonDocument doc(2048);
        JsonArray array = doc.to<JsonArray>();

        File root = LittleFS.open("/images");
        if (root && root.isDirectory()) {
            File file = root.openNextFile();
            while (file) {
                if (!file.isDirectory()) {
                    JsonObject obj = array.createNestedObject();
                    obj["name"] = String(file.name());
                    obj["path"] = "/images/" + String(file.name());
                    obj["size"] = file.size();
                }
                file = root.openNextFile();
            }
        }

        String output;
        serializeJson(doc, output);
        request->send(200, "application/json", output);
    });
}

void WebServerManager::handleGetCountdowns(AsyncWebServerRequest* request) {
    DynamicJsonDocument doc(4096);
    JsonArray array = doc.to<JsonArray>();

    std::vector<Countdown> countdowns = storage.getAllCountdowns();
    for (const auto& cd : countdowns) {
        JsonObject obj = array.createNestedObject();
        obj["uid"] = cd.uid;
        obj["name"] = cd.name;
        obj["targetDate"] = cd.targetDate;
        obj["imagePath"] = cd.imagePath;
        obj["active"] = cd.active;
    }

    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
}

void WebServerManager::handleDeleteCountdown(AsyncWebServerRequest* request) {
    Serial.println("DELETE Request empfangen");
    Serial.print("URL: ");
    Serial.println(request->url());

    String uid = request->url().substring(request->url().lastIndexOf('/') + 1);
    Serial.print("Extrahierte UID: ");
    Serial.println(uid);

    if (uid.length() == 0) {
        Serial.println("FEHLER: UID ist leer!");
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Keine UID angegeben\"}");
        return;
    }

    bool result = storage.deleteCountdown(uid);
    Serial.print("Delete Result: ");
    Serial.println(result ? "Erfolgreich" : "Fehlgeschlagen");

    if (result) {
        request->send(200, "application/json", "{\"success\":true}");
    } else {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Konnte Countdown nicht löschen\"}");
    }
}

void WebServerManager::handleGetWiFi(AsyncWebServerRequest* request) {
    String ssid, password;
    storage.getWiFiCredentials(ssid, password);

    DynamicJsonDocument doc(512);
    doc["ssid"] = ssid;
    doc["hasPassword"] = !password.isEmpty();
    doc["apMode"] = apMode;

    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
}

void WebServerManager::handleScanCard(AsyncWebServerRequest* request) {
    // Versuche zuerst, eine Karte zu lesen
    String uid = rfidReader.readCardUID();

    // Wenn keine neue Karte, verwende die zuletzt gelesene (innerhalb von 10 Sekunden)
    if (uid.length() == 0) {
        unsigned long lastTime = rfidReader.getLastReadTime();
        if (lastTime > 0 && (millis() - lastTime) < 10000) {  // 10 Sekunden Cache
            uid = rfidReader.getLastCardUID();
            Serial.println("Verwende gecachte Karten-UID für Web-Anfrage");
        }
    }

    if (uid.length() > 0) {
        DynamicJsonDocument doc(256);
        doc["success"] = true;
        doc["uid"] = uid;

        String output;
        serializeJson(doc, output);
        request->send(200, "application/json", output);
    } else {
        request->send(200, "application/json", "{\"success\":false,\"error\":\"Keine Karte gefunden. Bitte Karte nah an Leser halten.\"}");
    }
}
