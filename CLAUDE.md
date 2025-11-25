# CLAUDE.md - AI Assistant Guide

Diese Datei dient als Leitfaden für AI-Assistenten (wie Claude) beim Arbeiten mit diesem Projekt.

## Projektübersicht

**Countdown Display mit E-Ink und RFID** ist ein ESP32-basiertes System, das verschiedene Countdowns auf einem E-Ink Display anzeigt. Die Auswahl erfolgt durch RFID-Karten.

### Kern-Technologien
- **Platform**: ESP32 (Arduino Framework)
- **Build-System**: PlatformIO
- **Sprache**: C++ (Arduino)
- **Display**: Waveshare 7.5" E-Ink Display V2 (800x480)
- **RFID**: MFRC522 (13.56MHz)
- **Webserver**: ESPAsyncWebServer
- **Filesystem**: LittleFS
- **Datenformat**: JSON (ArduinoJson)

## Codebase-Struktur

```
Countdowndisplay/
├── src/
│   ├── main.cpp          # Hauptprogramm, Setup & Loop
│   ├── display.cpp       # E-Ink Display Management
│   ├── rfid.cpp          # RFID-Leser Logik
│   ├── webserver.cpp     # Webserver & REST API
│   └── storage.cpp       # LittleFS Datenspeicherung
├── include/
│   ├── config.h          # Pin-Definitionen & Konstanten
│   ├── display.h         # Display Manager Header
│   ├── rfid.h            # RFID Manager Header
│   ├── webserver.h       # Webserver Header
│   └── storage.h         # Storage Manager Header
├── data/                 # Webinterface Dateien (HTML/CSS/JS)
├── platformio.ini        # PlatformIO Konfiguration
└── README.md             # Benutzer-Dokumentation
```

## Architektur & Komponenten

### 1. Display Manager (`display.cpp` / `display.h`)
**Verantwortlichkeiten**:
- Initialisierung des E-Ink Displays (GxEPD2)
- Rendering von Countdowns mit deutschem Datumsformat
- Anzeige von Fehlermeldungen
- Bildunterstützung (optional)
- Energieeffiziente Display-Updates

**Wichtige Funktionen**:
- `void init()` - Display initialisieren
- `void showCountdown(const Countdown& countdown, int daysRemaining)` - Countdown anzeigen
- `void showError(const String& message)` - Fehlermeldung anzeigen
- `void clear()` - Display löschen

**Besonderheiten**:
- E-Ink benötigt nur beim Update Strom
- Vollständiger Refresh für beste Bildqualität
- Deutsche Monatsbezeichnungen hardcoded

### 2. RFID Manager (`rfid.cpp` / `rfid.h`)
**Verantwortlichkeiten**:
- RFID-Leser Initialisierung (MFRC522)
- Karten-Scanning
- UID-Konvertierung zu String

**Wichtige Funktionen**:
- `void init()` - RFID-Leser initialisieren
- `bool readCard(String& uid)` - Karte lesen und UID zurückgeben
- `bool isNewCardPresent()` - Prüfen ob Karte vorhanden

**Besonderheiten**:
- Teilt SPI-Bus mit E-Ink Display
- Separater CS Pin für Device-Selektion

### 3. Storage Manager (`storage.cpp` / `storage.h`)
**Verantwortlichkeiten**:
- LittleFS Filesystem Management
- JSON-Serialisierung/Deserialisierung
- Persistente Speicherung von Countdowns und WiFi-Einstellungen

**Datenstrukturen**:
```cpp
struct Countdown {
    String uid;           // RFID UID
    String name;          // Event-Name
    String targetDate;    // Zieldatum (YYYY-MM-DD)
    bool active;          // Aktiviert/Deaktiviert
};
```

**Wichtige Funktionen**:
- `bool loadConfig()` - Konfiguration laden
- `bool saveConfig()` - Konfiguration speichern
- `std::vector<Countdown> getCountdowns()` - Alle Countdowns abrufen
- `bool addCountdown(const Countdown& countdown)` - Countdown hinzufügen
- `bool deleteCountdown(const String& uid)` - Countdown löschen

**Speicherort**: `/config.json` im Flash

### 4. Webserver (`webserver.cpp` / `webserver.h`)
**Verantwortlichkeiten**:
- HTTP-Server (Port 80)
- REST API Endpunkte
- Webinterface Dateien aus LittleFS bereitstellen
- CORS-Headers für API-Zugriff

**REST API Endpunkte**:
```
GET    /api/countdowns           # Liste aller Countdowns
POST   /api/countdowns           # Neuen Countdown erstellen
PUT    /api/countdowns/:uid      # Countdown aktualisieren
DELETE /api/countdowns/:uid      # Countdown löschen
GET    /api/wifi                 # WiFi-Einstellungen abrufen
POST   /api/wifi                 # WiFi-Einstellungen setzen
GET    /api/scan-card            # RFID-Karte scannen
GET    /api/status               # System-Status
POST   /api/restart              # System-Neustart
```

**Request/Response Format**: JSON

### 5. Main Loop (`main.cpp`)
**Verantwortlichkeiten**:
- Initialisierung aller Subsysteme
- WiFi Management (AP/Client Modus)
- NTP Zeit-Synchronisation
- RFID-Karten-Überwachung
- Periodische Display-Updates

**WiFi-Modi**:
- **Access Point Modus**: Fallback wenn keine WiFi-Config oder Verbindung fehlschlägt
  - SSID: `CountdownDisplay`
  - Passwort: `countdown123`
  - IP: `192.168.4.1`
- **Client Modus**: Verbindung zu konfiguriertem WLAN

**Zeitzone**: MEZ (GMT+1) mit automatischer Sommerzeit

## Hardware-Konfiguration

### Pin-Definitionen (`include/config.h`)

**RFID RC522**:
```cpp
#define RFID_SS_PIN     21    // Chip Select
#define RFID_RST_PIN    22    // Reset
// SPI: SCK=18, MOSI=23, MISO=19 (geteilt)
```

**E-Ink Display**:
```cpp
#define DISPLAY_CS      5     // Chip Select
#define DISPLAY_DC      17    // Data/Command
#define DISPLAY_RST     16    // Reset
#define DISPLAY_BUSY    4     // Busy Signal
// SPI: SCK=18, MOSI=23 (geteilt mit RFID)
```

**Wichtig**: RFID und Display teilen den SPI-Bus!

## Bibliotheken & Abhängigkeiten

Definiert in `platformio.ini`:
- **GxEPD2** (1.5.8+): E-Ink Display Treiber
- **Adafruit GFX** (1.11.9+): Grafik-Bibliothek
- **MFRC522** (1.4.11+): RFID-Leser
- **ArduinoJson** (6.21.3+): JSON Parsing
- **ESPAsyncWebServer** (3.4.5+): Async Webserver
- **AsyncTCP** (3.2.14+): Async TCP Stack

## Entwicklungsrichtlinien

### Code-Stil
- **Header Guards**: `#ifndef COMPONENT_H` / `#define COMPONENT_H`
- **Namenskonventionen**:
  - Klassen: `PascalCase`
  - Funktionen: `camelCase`
  - Konstanten: `UPPER_SNAKE_CASE`
- **Debugging**: Serial.println() für Debug-Ausgaben (115200 baud)

### Häufige Entwicklungsaufgaben

#### Display-Layout ändern
Bearbeite `src/display.cpp::showCountdown()`:
- Fonts über Adafruit GFX
- Positionen als Pixel-Koordinaten (0,0 = oben links)
- Text-Alignment beachten

#### Neue API-Endpunkte hinzufügen
1. Handler in `src/webserver.cpp` erstellen
2. Route in `WebServerManager::setupRoutes()` registrieren
3. JSON Request/Response mit ArduinoJson

#### Countdown-Berechnungslogik anpassen
Siehe `src/main.cpp::loop()`:
```cpp
int daysRemaining = (targetTimestamp - currentTimestamp) / 86400;
```

#### Pin-Konfiguration ändern
Alle Pins in `include/config.h` definiert. Nach Änderung:
- Hardware entsprechend verkabeln
- Neu kompilieren und uploaden

### Build & Deploy

**Kompilieren**:
```bash
pio run
```

**Upload (Code)**:
```bash
pio run -t upload
```

**Upload (Webinterface/Filesystem)**:
```bash
pio run -t uploadfs
```

**Serial Monitor**:
```bash
pio device monitor
```

### Testing & Debugging

**Serial Monitor beachten**:
- WiFi-Status und IP-Adresse
- RFID-Karten UIDs beim Scannen
- NTP-Synchronisation
- Fehlermeldungen

**Häufige Debug-Punkte**:
- Display-Initialisierung: "Display initialized" Nachricht
- RFID-Leser: "RFID initialized" Nachricht
- WiFi-Verbindung: IP-Adresse oder AP-Modus
- Zeit-Sync: "Time synced" Nachricht

## JSON Datenformat

### Config-Datei (`/config.json`)
```json
{
  "wifi": {
    "ssid": "MeinWLAN",
    "password": "passwort123"
  },
  "countdowns": [
    {
      "uid": "A1B2C3D4",
      "name": "Laras Geburtstag",
      "targetDate": "2025-06-15",
      "active": true
    }
  ]
}
```

## Bekannte Einschränkungen

1. **E-Ink Refresh-Zeit**: ~2-3 Sekunden für vollständigen Update
2. **RFID-Reichweite**: ~3-5 cm je nach Karte
3. **WiFi**: Nur 2.4 GHz (ESP32-Limit)
4. **Zeitzone**: Hardcoded auf MEZ/MESZ
5. **Maximale Countdowns**: Begrenzt durch Flash-Speicher (~4MB)
6. **Display-Größe**: Hardcoded für 7.5" (800x480)

## Wichtige Hinweise für AI-Assistenten

### Bei Code-Änderungen
1. **Immer vorhandene Dateien bevorzugen**: Nutze Edit statt Write für existierende Dateien
2. **Pin-Sharing beachten**: RFID und Display teilen SPI - CS Pins sind kritisch!
3. **E-Ink Besonderheiten**: Vollständiger Refresh für beste Qualität, keine Partial Updates implementiert
4. **Speicher-Management**: JSON-Dokumente sind stack-allocated, auf Größe achten
5. **Async Webserver**: Callbacks müssen schnell sein, keine blocking operations

### Bei Hardware-Fragen
- Referenziere `README.md` für Pin-Belegung
- Waveshare E-Ink hat spezifische Initialisierung (siehe GxEPD2 Docs)
- RFID RC522 benötigt 3.3V, nicht 5V!

### Bei Feature-Requests
- Prüfe erst vorhandene Funktionalität in bestehenden Dateien
- API-Erweiterungen sollten RESTful sein
- Berücksichtige Flash-Speicher-Limits für Webinterface

### Bei Bugs
1. Check Serial Monitor Output
2. Prüfe Pin-Konfiguration in `config.h`
3. Validiere JSON-Format in `/config.json`
4. Teste WiFi-Verbindung (2.4 GHz!)
5. Überprüfe Library-Versionen in `platformio.ini`

## Weitere Dokumentation

- **README.md**: Benutzer-Dokumentation, Setup-Anleitung
- **WAVESHARE_ADAPTER.md**: Waveshare Adapter Details (falls vorhanden)
- **TROUBLESHOOTING.md**: Häufige Probleme und Lösungen
- **DISPLAY_VERSION.md**: Display-Version Details (falls vorhanden)

## Letzte Aktualisierung

Dieses Dokument wurde erstellt für die aktuelle Version des Projekts (Stand: November 2025).

Bei strukturellen Änderungen am Projekt sollte diese Datei aktualisiert werden.
