#ifndef CONFIG_H
#define CONFIG_H

// RFID RC522 Pins (an den Header-Pins des Waveshare Boards)
#define RFID_SS_PIN     21
#define RFID_RST_PIN    22
#define RFID_SCK_PIN    18
#define RFID_MOSI_PIN   23
#define RFID_MISO_PIN   19

// E-Ink Display Pins (Waveshare E-Paper ESP32 Driver Board)
// Diese Pins sind fest auf dem Board verdrahtet!
#define EPD_CS_PIN      15    // CS
#define EPD_DC_PIN      27    // DC
#define EPD_RST_PIN     26    // RST
#define EPD_BUSY_PIN    25    // BUSY
#define EPD_SCK_PIN     13    // CLK
#define EPD_MOSI_PIN    14    // DIN

// WiFi Settings (werden über Webinterface konfiguriert)
#define WIFI_SSID       "CountdownDisplay"
#define WIFI_PASSWORD   "countdown123"

// Maximum number of countdowns
#define MAX_COUNTDOWNS  20

// Storage file
#define CONFIG_FILE     "/config.json"

#endif
