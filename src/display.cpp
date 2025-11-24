#include "display.h"
#include "config.h"
#include <time.h>

DisplayManager displayManager;

DisplayManager::DisplayManager() {
    // GxEPD2_750_T7: Waveshare 7.5" V2 (800x480)
    display = new GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT>(
        GxEPD2_750_T7(EPD_CS_PIN, EPD_DC_PIN, EPD_RST_PIN, EPD_BUSY_PIN)
    );
}

bool DisplayManager::begin() {
    display->init(115200);
    display->setRotation(0);
    display->setTextColor(GxEPD_BLACK);

    Serial.println("E-Ink Display initialisiert");
    return true;
}

void DisplayManager::showWelcomeScreen() {
    display->setFullWindow();
    display->firstPage();
    do {
        display->fillScreen(GxEPD_WHITE);
        drawBorder();

        display->setFont(&FreeSansBold24pt7b);
        drawCenteredText("Countdown Display", 200, &FreeSansBold24pt7b);

        display->setFont(&FreeSans12pt7b);
        drawCenteredText("Bitte RFID Karte vorhalten", 280, &FreeSans12pt7b);

        display->setFont(&FreeSans9pt7b);
        drawCenteredText("Zum Konfigurieren mit WiFi verbinden", 350, &FreeSans9pt7b);
    } while (display->nextPage());
}

void DisplayManager::showCountdown(const Countdown& countdown, int daysRemaining) {
    display->setFullWindow();
    display->firstPage();
    do {
        display->fillScreen(GxEPD_WHITE);
        drawBorder();

        // Titel
        display->setFont(&FreeSansBold18pt7b);
        drawCenteredText(countdown.name, 100, &FreeSansBold18pt7b);

        // Tage verbleibend - große Anzeige
        display->setFont(&FreeSansBold24pt7b);
        String daysText = String(abs(daysRemaining));
        int16_t x1, y1;
        uint16_t w, h;
        display->getTextBounds(daysText, 0, 0, &x1, &y1, &w, &h);
        display->setCursor((800 - w) / 2, 240);
        display->print(daysText);

        // "Tage" Text
        display->setFont(&FreeSansBold18pt7b);
        String labelText = "Tage";
        if (daysRemaining < 0) {
            labelText = "Tage her";
        } else if (daysRemaining == 0) {
            labelText = "Heute!";
        } else if (daysRemaining == 1) {
            labelText = "Tag";
        }
        drawCenteredText(labelText, 290, &FreeSansBold18pt7b);

        // Zieldatum
        display->setFont(&FreeSans12pt7b);
        drawCenteredText("Datum: " + countdown.targetDate, 360, &FreeSans12pt7b);

    } while (display->nextPage());
}

void DisplayManager::showError(const String& message) {
    display->setFullWindow();
    display->firstPage();
    do {
        display->fillScreen(GxEPD_WHITE);
        drawBorder();

        display->setFont(&FreeSansBold18pt7b);
        drawCenteredText("Fehler", 200, &FreeSansBold18pt7b);

        display->setFont(&FreeSans12pt7b);
        drawCenteredText(message, 280, &FreeSans12pt7b);
    } while (display->nextPage());
}

void DisplayManager::showNoCardScreen() {
    display->setFullWindow();
    display->firstPage();
    do {
        display->fillScreen(GxEPD_WHITE);
        drawBorder();

        display->setFont(&FreeSansBold18pt7b);
        drawCenteredText("Keine Karte zugeordnet", 220, &FreeSansBold18pt7b);

        display->setFont(&FreeSans12pt7b);
        drawCenteredText("Bitte Karte im Webinterface", 280, &FreeSans12pt7b);
        drawCenteredText("konfigurieren", 320, &FreeSans12pt7b);
    } while (display->nextPage());
}

void DisplayManager::clear() {
    display->clearScreen();
}

int DisplayManager::calculateDaysRemaining(const String& targetDate) {
    // Parse target date (Format: YYYY-MM-DD)
    int year, month, day;
    if (sscanf(targetDate.c_str(), "%d-%d-%d", &year, &month, &day) != 3) {
        return -9999; // Fehler
    }

    // Aktuelles Datum
    time_t now;
    time(&now);
    struct tm* timeinfo = localtime(&now);

    // Berechne Zeitdifferenz
    struct tm target = {0};
    target.tm_year = year - 1900;
    target.tm_mon = month - 1;
    target.tm_mday = day;
    target.tm_hour = 0;
    target.tm_min = 0;
    target.tm_sec = 0;

    // Konvertiere zu time_t
    time_t targetTime = mktime(&target);

    // Setze aktuelle Zeit auf Mitternacht
    struct tm currentDay = *timeinfo;
    currentDay.tm_hour = 0;
    currentDay.tm_min = 0;
    currentDay.tm_sec = 0;
    time_t currentTime = mktime(&currentDay);

    // Berechne Differenz in Tagen
    double diffSeconds = difftime(targetTime, currentTime);
    int days = (int)(diffSeconds / 86400.0); // 86400 Sekunden pro Tag

    return days;
}

void DisplayManager::drawCenteredText(const String& text, int y, const GFXfont* font) {
    display->setFont(font);
    int16_t x1, y1;
    uint16_t w, h;
    display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    display->setCursor((800 - w) / 2, y);
    display->print(text);
}

void DisplayManager::drawBorder() {
    display->drawRect(10, 10, 780, 460, GxEPD_BLACK);
    display->drawRect(12, 12, 776, 456, GxEPD_BLACK);
}
