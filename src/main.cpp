#include <Arduino.h>
#include "config.h"
#include "storage.h"
#include "rfid.h"
#include "display.h"
#include "webserver.h"

// State Management
String currentCardUID = "";
Countdown* currentCountdown = nullptr;
unsigned long lastCardCheck = 0;
unsigned long lastDisplayUpdate = 0;
bool displayNeedsUpdate = true;

const unsigned long CARD_CHECK_INTERVAL = 1000;    // Prüfe alle 1 Sekunde auf Karte
const unsigned long DISPLAY_UPDATE_INTERVAL = 3600000; // Update Display alle Stunde (für Datum-Änderung)

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n=================================");
    Serial.println("   Countdown Display System");
    Serial.println("=================================\n");

    // Initialisiere Storage (LittleFS)
    Serial.println("Initialisiere Speicher...");
    if (!storage.begin()) {
        Serial.println("FEHLER: Storage konnte nicht initialisiert werden!");
        while (1) delay(1000);
    }

    // Initialisiere RFID
    Serial.println("Initialisiere RFID Reader...");
    if (!rfidReader.begin()) {
        Serial.println("FEHLER: RFID Reader konnte nicht initialisiert werden!");
        while (1) delay(1000);
    }

    // Initialisiere Display
    Serial.println("Initialisiere E-Ink Display...");
    if (!displayManager.begin()) {
        Serial.println("FEHLER: Display konnte nicht initialisiert werden!");
        while (1) delay(1000);
    }

    // Zeige Willkommensbildschirm
    displayManager.showWelcomeScreen();

    // Initialisiere Webserver
    Serial.println("Initialisiere Webserver...");
    if (!webServer.begin()) {
        Serial.println("FEHLER: Webserver konnte nicht gestartet werden!");
    }

    // Konfiguriere Zeit (NTP)
    configTime(3600, 3600, "pool.ntp.org", "time.nist.gov"); // MEZ + Sommerzeit
    Serial.println("Warte auf NTP Zeit-Synchronisation...");

    // Warte auf Zeit-Sync (max 10 Sekunden)
    int ntpWait = 0;
    while (time(nullptr) < 100000 && ntpWait < 20) {
        delay(500);
        Serial.print(".");
        ntpWait++;
    }
    Serial.println();

    if (time(nullptr) < 100000) {
        Serial.println("WARNUNG: NTP Zeit-Synchronisation fehlgeschlagen!");
    } else {
        time_t now = time(nullptr);
        Serial.print("Aktuelle Zeit: ");
        Serial.println(ctime(&now));
    }

    Serial.println("\n=================================");
    Serial.println("System bereit!");
    Serial.println("=================================\n");
    Serial.print("Webinterface: http://");
    Serial.println(webServer.getIPAddress());
    Serial.println();
}

void loop() {
    unsigned long currentMillis = millis();

    // Prüfe regelmäßig auf neue RFID Karte
    if (currentMillis - lastCardCheck >= CARD_CHECK_INTERVAL) {
        lastCardCheck = currentMillis;

        String uid = rfidReader.readCardUID();

        // Wenn eine neue Karte erkannt wurde
        if (uid.length() > 0 && uid != currentCardUID) {
            currentCardUID = uid;
            Serial.print("Neue Karte erkannt: ");
            Serial.println(uid);

            // Suche entsprechenden Countdown
            currentCountdown = storage.getCountdownByUID(uid);

            if (currentCountdown != nullptr) {
                Serial.print("Countdown gefunden: ");
                Serial.println(currentCountdown->name);
                displayNeedsUpdate = true;
            } else {
                Serial.println("Keine Konfiguration für diese Karte gefunden");
                displayManager.showNoCardScreen();
                displayNeedsUpdate = false;
            }
        }
    }

    // Update Display wenn nötig
    if (displayNeedsUpdate && currentCountdown != nullptr) {
        if (currentMillis - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL || lastDisplayUpdate == 0) {
            lastDisplayUpdate = currentMillis;

            int daysRemaining = displayManager.calculateDaysRemaining(currentCountdown->targetDate);

            if (daysRemaining == -9999) {
                displayManager.showError("Ungültiges Datum");
            } else {
                Serial.print("Zeige Countdown: ");
                Serial.print(currentCountdown->name);
                Serial.print(" - Tage verbleibend: ");
                Serial.println(daysRemaining);

                displayManager.showCountdown(*currentCountdown, daysRemaining);
            }

            // Nach dem ersten Update, update nur noch täglich
            displayNeedsUpdate = false;
        }
    }

    // Webserver läuft asynchron
    webServer.handle();

    // Kleine Pause
    delay(10);
}
