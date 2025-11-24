#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include "storage.h"

// Waveshare 7.5" V2 Display
// Größe: 800x480 pixels
class DisplayManager {
public:
    DisplayManager();
    bool begin();

    void showWelcomeScreen();
    void showCountdown(const Countdown& countdown, int daysRemaining);
    void showError(const String& message);
    void showNoCardScreen();
    void clear();

    int calculateDaysRemaining(const String& targetDate);

private:
    GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT>* display;

    void drawCenteredText(const String& text, int y, const GFXfont* font);
    void drawBorder();
};

extern DisplayManager displayManager;

#endif
