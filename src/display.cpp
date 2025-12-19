#include "display.h"
#include "config.h"
#include <time.h>
#include <LittleFS.h>

DisplayManager displayManager;

DisplayManager::DisplayManager() {
    // GxEPD2_750_T7: Waveshare 7.5" V2 (800x480)
    // Verwende HSPI-Bus für das Waveshare E-Paper ESP32 Driver Board
    display = new GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT>(
        GxEPD2_750_T7(EPD_CS_PIN, EPD_DC_PIN, EPD_RST_PIN, EPD_BUSY_PIN)
    );
}

bool DisplayManager::begin() {
    // Setze HSPI als SPI-Bus für das Display
    display->epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));

    // Initialisiere Display (HSPI ist bereits in main.cpp initialisiert)
    display->init(0, true, 2, false); // (serial_diag, initial, reset_duration, pulldown_rst)
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

        // Titel - immer mittig und oben (größerer Font)
        display->setFont(&FreeSansBold24pt7b);
        drawCenteredText(countdown.name, 80, &FreeSansBold24pt7b);

        // Prüfe ob Bild vorhanden
        bool hasImage = false;
        if (countdown.imagePath.length() > 0) {
            Serial.print("Versuche Bild zu laden: ");
            Serial.println(countdown.imagePath);
            // Bild links unten anzeigen (bündig mit Datumszeile)
            // Position: x=60, y beginnt bei 180 (damit es bis zur Datumszeile bei ~430 reicht)
            hasImage = drawBMPImage(countdown.imagePath, 60, 180, 250, 250);
            if (hasImage) {
                Serial.println("✓ Bild erfolgreich geladen und gezeichnet");
            } else {
                Serial.println("✗ Bild konnte nicht geladen werden");
            }
        } else {
            Serial.println("Kein Bildpfad angegeben - zeige nur Text");
        }

        int16_t x1, y1;
        uint16_t w, h;

        if (hasImage) {
            // Layout mit Bild: Text rechts vom Bild
            int textAreaX = 350;      // Startposition für Text rechts vom Bild
            int textAreaWidth = 400;  // Verfügbare Breite für Text

            // Tage verbleibend - sehr große Anzeige
            display->setFont(&FreeSansBold24pt7b);
            String daysText = String(abs(daysRemaining));
            display->getTextBounds(daysText, 0, 0, &x1, &y1, &w, &h);
            display->setCursor(textAreaX + (textAreaWidth - w) / 2, 240);
            display->print(daysText);

            // "Tage" Label (größerer Font)
            display->setFont(&FreeSansBold18pt7b);
            String labelText = "Tage";
            if (daysRemaining < 0) {
                labelText = "Tage her";
            } else if (daysRemaining == 0) {
                labelText = "Heute!";
            } else if (daysRemaining == 1) {
                labelText = "Tag";
            }
            display->getTextBounds(labelText, 0, 0, &x1, &y1, &w, &h);
            display->setCursor(textAreaX + (textAreaWidth - w) / 2, 300);
            display->print(labelText);

            // Datum (größerer Font, bündig mit Bildunterkante)
            display->setFont(&FreeSans18pt7b);
            String dateStr = formatDateGerman(countdown.targetDate);
            display->getTextBounds(dateStr, 0, 0, &x1, &y1, &w, &h);
            display->setCursor(textAreaX + (textAreaWidth - w) / 2, 420);
            display->print(dateStr);

        } else {
            // Layout ohne Bild: Alles zentriert, größere Fonts

            // Tage verbleibend - extra große Anzeige
            display->setFont(&FreeSansBold24pt7b);
            String daysText = String(abs(daysRemaining));
            display->getTextBounds(daysText, 0, 0, &x1, &y1, &w, &h);
            display->setCursor((800 - w) / 2, 260);
            display->print(daysText);

            // "Tage" Label (größerer Font)
            display->setFont(&FreeSansBold18pt7b);
            String labelText = "Tage";
            if (daysRemaining < 0) {
                labelText = "Tage her";
            } else if (daysRemaining == 0) {
                labelText = "Heute!";
            } else if (daysRemaining == 1) {
                labelText = "Tag";
            }
            drawCenteredText(labelText, 330, &FreeSansBold18pt7b);

            // Datum (größerer Font)
            display->setFont(&FreeSans18pt7b);
            String dateStr = formatDateGerman(countdown.targetDate);
            drawCenteredText(dateStr, 390, &FreeSans18pt7b);
        }

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

String DisplayManager::formatDateGerman(const String& date) {
    // Format: YYYY-MM-DD -> DD.MM.YYYY
    int year, month, day;
    if (sscanf(date.c_str(), "%d-%d-%d", &year, &month, &day) != 3) {
        return date; // Bei Fehler: Original zurückgeben
    }

    char buffer[11]; // DD.MM.YYYY + \0
    snprintf(buffer, sizeof(buffer), "%02d.%02d.%04d", day, month, year);
    return String(buffer);
}

bool DisplayManager::drawBMPImage(const String& filename, int16_t x, int16_t y, int16_t maxWidth, int16_t maxHeight) {
    // Öffne Datei
    if (!LittleFS.exists(filename)) {
        Serial.println("Bild nicht gefunden: " + filename);
        return false;
    }

    File file = LittleFS.open(filename, "r");
    if (!file) {
        Serial.println("Fehler beim Öffnen der Bilddatei");
        return false;
    }

    // Lese BMP Header (14 Bytes)
    uint8_t bmpHeader[14];
    if (file.read(bmpHeader, 14) != 14) {
        file.close();
        return false;
    }

    // Prüfe BMP Signatur
    if (bmpHeader[0] != 'B' || bmpHeader[1] != 'M') {
        Serial.println("Keine gültige BMP-Datei");
        file.close();
        return false;
    }

    // Lese DIB Header (mindestens 40 Bytes)
    uint8_t dibHeader[40];
    if (file.read(dibHeader, 40) != 40) {
        file.close();
        return false;
    }

    // Extrahiere Bildinformationen
    int32_t width = *(int32_t*)(dibHeader + 4);
    int32_t height = *(int32_t*)(dibHeader + 8);
    uint16_t bitsPerPixel = *(uint16_t*)(dibHeader + 14);
    uint32_t imageOffset = *(uint32_t*)(bmpHeader + 10);

    Serial.print("BMP Info: ");
    Serial.print(width);
    Serial.print("x");
    Serial.print(abs(height));
    Serial.print(" Pixel, ");
    Serial.print(bitsPerPixel);
    Serial.println(" Bits pro Pixel");

    // Prüfe, ob Bild monochrom ist (1 Bit pro Pixel)
    if (bitsPerPixel != 1) {
        Serial.print("FEHLER: Bild hat ");
        Serial.print(bitsPerPixel);
        Serial.println(" Bits pro Pixel. Nur 1-bit (monochrom) wird unterstützt!");
        Serial.println("Bitte konvertiere das Bild zu 1-bit monochrom BMP");
        file.close();
        return false;
    }

    // Berechne skalierte Größe (falls nötig)
    int16_t displayWidth = width;
    int16_t displayHeight = abs(height);

    if (displayWidth > maxWidth || displayHeight > maxHeight) {
        float scaleX = (float)maxWidth / displayWidth;
        float scaleY = (float)maxHeight / displayHeight;
        float scale = (scaleX < scaleY) ? scaleX : scaleY;
        displayWidth = (int16_t)(displayWidth * scale);
        displayHeight = (int16_t)(displayHeight * scale);
    }

    // Für einfache Implementierung: Zeichne das Bild 1:1 ohne Skalierung
    // (Skalierung würde mehr Code erfordern)
    if (width > maxWidth) width = maxWidth;
    if (abs(height) > maxHeight) height = (height > 0) ? maxHeight : -maxHeight;

    // Gehe zum Bildanfang
    file.seek(imageOffset);

    // BMP ist von unten nach oben gespeichert
    bool topDown = (height < 0);
    if (topDown) height = -height;

    // Berechne Row-Padding (BMP Zeilen sind auf 4 Bytes ausgerichtet)
    int rowSize = ((width + 31) / 32) * 4;

    // Zeichne Bild Zeile für Zeile
    uint8_t* rowBuffer = (uint8_t*)malloc(rowSize);
    if (!rowBuffer) {
        file.close();
        return false;
    }

    for (int16_t row = 0; row < height; row++) {
        // Lese Zeile
        file.read(rowBuffer, rowSize);

        int16_t drawY = topDown ? (y + row) : (y + height - 1 - row);

        // Zeichne Pixel
        for (int16_t col = 0; col < width; col++) {
            int byteIndex = col / 8;
            int bitIndex = 7 - (col % 8);
            bool pixelSet = (rowBuffer[byteIndex] >> bitIndex) & 1;

            // Schwarz zeichnen (invertiert, da BMP 0=schwarz, 1=weiß bei monochromen Bildern oft umgekehrt ist)
            if (!pixelSet) {
                display->drawPixel(x + col, drawY, GxEPD_BLACK);
            }
        }
    }

    free(rowBuffer);
    file.close();

    Serial.println("Bild erfolgreich gezeichnet: " + filename);
    return true;
}
