#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>

struct Countdown {
    String uid;           // RFID UID (8 bytes hex string)
    String name;          // Name (z.B. "Laras Geburtstag")
    String targetDate;    // Datum im Format "YYYY-MM-DD"
    bool active;          // Ist dieser Countdown aktiv?
};

class StorageManager {
public:
    StorageManager();
    bool begin();

    // Countdown Management
    bool addCountdown(const Countdown& countdown);
    bool updateCountdown(const String& uid, const Countdown& countdown);
    bool deleteCountdown(const String& uid);
    Countdown* getCountdownByUID(const String& uid);
    std::vector<Countdown> getAllCountdowns();

    // WiFi Settings
    bool saveWiFiCredentials(const String& ssid, const String& password);
    bool getWiFiCredentials(String& ssid, String& password);

    // Save/Load
    bool saveToFile();
    bool loadFromFile();

private:
    std::vector<Countdown> countdowns;
    String wifiSSID;
    String wifiPassword;

    String serializeToJson();
    bool deserializeFromJson(const String& json);
};

extern StorageManager storage;

#endif
