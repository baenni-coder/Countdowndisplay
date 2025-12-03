#include <Arduino.h>
#include <SPI.h>
#include "config.h"
#include "storage.h"
#include "rfid.h"
#include "display.h"
#include "webserver.h"

// Separate SPI-Busse für das Waveshare E-Paper ESP32 Driver Board
SPIClass hspi(HSPI);  // Display (GPIO 13, 14)
SPIClass vspi(VSPI);  // RFID (GPIO 18, 19, 23)

// State Management
String currentCardUID = "";
Countdown* currentCountdown = nullptr;
unsigned long lastCardCheck = 0;
unsigned long lastDisplayUpdate = 0;
int lastUpdateDay = -1;  // Speichert den Tag der letzten Aktualisierung (für Mitternachts-Check)
bool displayNeedsUpdate = true;

const unsigned long CARD_CHECK_INTERVAL = 1000;    // Prüfe alle 1 Sekunde auf Karte
const unsigned long MIDNIGHT_CHECK_INTERVAL = 60000; // Prüfe alle 60 Sekunden auf Mitternacht

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n=================================");
    Serial.println("   Countdown Display System");
    Serial.println("=================================\n");

    // Initialisiere Standard-SPI (VSPI) für RFID (GPIO 18, 19, 23)
    Serial.println("Initialisiere RFID SPI Bus (VSPI)...");
    SPI.begin(RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN, RFID_SS_PIN);
    Serial.println("RFID SPI Bus initialisiert");

    // Initialisiere HSPI für Display (GPIO 13, 14) - wird im Display-Code verwendet
    Serial.println("Initialisiere Display SPI Bus (HSPI)...");
    hspi.begin(EPD_SCK_PIN, -1, EPD_MOSI_PIN, EPD_CS_PIN);
    Serial.println("Display SPI Bus initialisiert");

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

        // Wenn keine Karte erkannt wurde, zurücksetzen
        if (uid.length() == 0) {
            if (currentCardUID.length() > 0) {
                Serial.println("Karte entfernt");
                currentCardUID = "";
                currentCountdown = nullptr;
            }
        }
        // Wenn eine neue Karte erkannt wurde
        else if (uid != currentCardUID) {
            currentCardUID = uid;
            Serial.print("Neue Karte erkannt: ");
            Serial.println(uid);

            // Suche entsprechenden Countdown
            currentCountdown = storage.getCountdownByUID(uid);

            if (currentCountdown != nullptr) {
                Serial.print("Countdown gefunden: ");
                Serial.println(currentCountdown->name);

                // SOFORT Display aktualisieren bei neuer Karte
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

                lastDisplayUpdate = currentMillis;  // Zeitstempel aktualisieren

                // Speichere aktuellen Tag für Mitternachts-Check
                time_t now = time(nullptr);
                struct tm* timeinfo = localtime(&now);
                lastUpdateDay = timeinfo->tm_mday;

                displayNeedsUpdate = false;
            } else {
                Serial.println("Keine Konfiguration für diese Karte gefunden");
                displayManager.showNoCardScreen();
                displayNeedsUpdate = false;
            }
        }
    }

    // Mitternachts-Update: Prüfe ob neuer Tag begonnen hat
    if (currentCountdown != nullptr && !displayNeedsUpdate) {
        // Prüfe alle 60 Sekunden auf Tageswechsel
        if (currentMillis - lastDisplayUpdate >= MIDNIGHT_CHECK_INTERVAL) {
            lastDisplayUpdate = currentMillis;

            // Hole aktuelle Zeit
            time_t now = time(nullptr);
            struct tm* timeinfo = localtime(&now);
            int currentDay = timeinfo->tm_mday;

            // Wenn der Tag sich geändert hat (nach Mitternacht)
            if (lastUpdateDay != -1 && currentDay != lastUpdateDay) {
                lastUpdateDay = currentDay;

                int daysRemaining = displayManager.calculateDaysRemaining(currentCountdown->targetDate);

                if (daysRemaining != -9999) {
                    Serial.println("🌙 Mitternachts-Update: Neuer Tag erkannt!");
                    Serial.print("   Datum: ");
                    Serial.print(timeinfo->tm_mday);
                    Serial.print(".");
                    Serial.print(timeinfo->tm_mon + 1);
                    Serial.print(".");
                    Serial.println(timeinfo->tm_year + 1900);
                    Serial.print("   Aktualisiere Countdown: ");
                    Serial.println(currentCountdown->name);

                    displayManager.showCountdown(*currentCountdown, daysRemaining);
                }
            }
        }
    }

    // Webserver läuft asynchron
    webServer.handle();

    // Kleine Pause
    delay(10);
}
