#include "storage.h"
#include "config.h"
#include <LittleFS.h>

StorageManager storage;

StorageManager::StorageManager() {
}

bool StorageManager::begin() {
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS mount fehlgeschlagen!");
        return false;
    }

    Serial.println("LittleFS erfolgreich gemountet");

    // Lade gespeicherte Konfiguration
    loadFromFile();

    return true;
}

bool StorageManager::addCountdown(const Countdown& countdown) {
    // Prüfe ob UID bereits existiert
    for (auto& cd : countdowns) {
        if (cd.uid == countdown.uid) {
            Serial.println("UID existiert bereits!");
            return false;
        }
    }

    // Prüfe maximale Anzahl
    if (countdowns.size() >= MAX_COUNTDOWNS) {
        Serial.println("Maximale Anzahl an Countdowns erreicht!");
        return false;
    }

    countdowns.push_back(countdown);
    return saveToFile();
}

bool StorageManager::updateCountdown(const String& uid, const Countdown& countdown) {
    for (auto& cd : countdowns) {
        if (cd.uid == uid) {
            cd = countdown;
            return saveToFile();
        }
    }
    return false;
}

bool StorageManager::deleteCountdown(const String& uid) {
    for (size_t i = 0; i < countdowns.size(); i++) {
        if (countdowns[i].uid == uid) {
            countdowns.erase(countdowns.begin() + i);
            return saveToFile();
        }
    }
    return false;
}

Countdown* StorageManager::getCountdownByUID(const String& uid) {
    for (auto& cd : countdowns) {
        if (cd.uid == uid && cd.active) {
            return &cd;
        }
    }
    return nullptr;
}

std::vector<Countdown> StorageManager::getAllCountdowns() {
    return countdowns;
}

bool StorageManager::saveWiFiCredentials(const String& ssid, const String& password) {
    wifiSSID = ssid;
    wifiPassword = password;
    return saveToFile();
}

bool StorageManager::getWiFiCredentials(String& ssid, String& password) {
    ssid = wifiSSID;
    password = wifiPassword;
    return (!wifiSSID.isEmpty());
}

bool StorageManager::saveToFile() {
    String json = serializeToJson();

    File file = LittleFS.open(CONFIG_FILE, "w");
    if (!file) {
        Serial.println("Fehler beim Öffnen der Config-Datei zum Schreiben!");
        return false;
    }

    file.print(json);
    file.close();

    Serial.println("Konfiguration gespeichert");
    return true;
}

bool StorageManager::loadFromFile() {
    if (!LittleFS.exists(CONFIG_FILE)) {
        Serial.println("Config-Datei existiert nicht, erstelle neue");
        return saveToFile();
    }

    File file = LittleFS.open(CONFIG_FILE, "r");
    if (!file) {
        Serial.println("Fehler beim Öffnen der Config-Datei!");
        return false;
    }

    String json = file.readString();
    file.close();

    bool result = deserializeFromJson(json);
    Serial.println("Konfiguration geladen");
    return result;
}

String StorageManager::serializeToJson() {
    JsonDocument doc;

    // WiFi Einstellungen
    doc["wifi"]["ssid"] = wifiSSID;
    doc["wifi"]["password"] = wifiPassword;

    // Countdowns
    JsonArray cdArray = doc["countdowns"].to<JsonArray>();
    for (const auto& cd : countdowns) {
        JsonObject cdObj = cdArray.add<JsonObject>();
        cdObj["uid"] = cd.uid;
        cdObj["name"] = cd.name;
        cdObj["targetDate"] = cd.targetDate;
        cdObj["active"] = cd.active;
    }

    String output;
    serializeJson(doc, output);
    return output;
}

bool StorageManager::deserializeFromJson(const String& json) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        Serial.print("JSON Parse Fehler: ");
        Serial.println(error.c_str());
        return false;
    }

    // WiFi Einstellungen
    wifiSSID = doc["wifi"]["ssid"].as<String>();
    wifiPassword = doc["wifi"]["password"].as<String>();

    // Countdowns
    countdowns.clear();
    JsonArray cdArray = doc["countdowns"].as<JsonArray>();
    for (JsonObject cdObj : cdArray) {
        Countdown cd;
        cd.uid = cdObj["uid"].as<String>();
        cd.name = cdObj["name"].as<String>();
        cd.targetDate = cdObj["targetDate"].as<String>();
        cd.active = cdObj["active"].as<bool>();
        countdowns.push_back(cd);
    }

    return true;
}
