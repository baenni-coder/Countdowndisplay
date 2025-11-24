#ifndef CONFIG_H
#define CONFIG_H

// RFID RC522 Pins
#define RFID_SS_PIN     21
#define RFID_RST_PIN    22
#define RFID_SCK_PIN    18
#define RFID_MOSI_PIN   23
#define RFID_MISO_PIN   19

// E-Ink Display Pins (Waveshare e-Paper Adapter)
#define EPD_CS_PIN      5
#define EPD_DC_PIN      17
#define EPD_RST_PIN     16
#define EPD_BUSY_PIN    4
#define EPD_SCK_PIN     18  // Shared with RFID
#define EPD_MOSI_PIN    23  // Shared with RFID

// WiFi Settings (werden über Webinterface konfiguriert)
#define WIFI_SSID       "CountdownDisplay"
#define WIFI_PASSWORD   "countdown123"

// Maximum number of countdowns
#define MAX_COUNTDOWNS  20

// Storage file
#define CONFIG_FILE     "/config.json"

#endif
