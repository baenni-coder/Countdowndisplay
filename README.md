# 📅 Countdown Display mit E-Ink und RFID

Ein interaktives Countdown-Display-System für ESP32 mit E-Ink Display und RFID-Kartenleser.

## 🎯 Projektbeschreibung

Dieses Projekt ermöglicht es, verschiedene Countdowns auf einem E-Ink Display anzuzeigen, wobei der angezeigte Countdown durch das Vorhalten einer RFID-Karte gewählt wird. Perfekt für Geburtstage, Urlaube oder andere wichtige Ereignisse!

### Features

- ⏱️ **Countdown in Tagen** bis zu wichtigen Ereignissen
- 🏷️ **RFID-gesteuerte Auswahl** verschiedener Countdowns
- 🖥️ **E-Ink Display** für energieeffiziente, augenschonende Anzeige
- 🌐 **Webinterface** zur einfachen Konfiguration
- 📡 **WiFi-Unterstützung** (Access Point & Client Modus)
- 💾 **Persistente Speicherung** aller Einstellungen
- 🕐 **Automatische Zeitsynchronisation** via NTP

## 🛠️ Hardware-Komponenten

### Benötigte Teile

- **ESP32 Development Board** (z.B. ESP32-DevKitC)
- **Waveshare 7.5" E-Ink Display V2** (800x480 Pixel)
- **Waveshare e-Paper Adapter** für ESP32
- **RFID RC522 Modul** (13.56MHz)
- **RFID Karten/Tags** (z.B. Mifare)
- **Stromversorgung** (USB oder 5V Adapter)

### Pin-Belegung

#### RFID RC522 ➔ ESP32

| RC522 Pin | ESP32 Pin | Funktion |
|-----------|-----------|----------|
| SDA/SS    | GPIO 21   | Chip Select |
| SCK       | GPIO 18   | SPI Clock (geteilt mit E-Ink) |
| MOSI      | GPIO 23   | SPI MOSI (geteilt mit E-Ink) |
| MISO      | GPIO 19   | SPI MISO |
| IRQ       | -         | Nicht verbunden |
| GND       | GND       | Ground |
| RST       | GPIO 22   | Reset |
| 3.3V      | 3.3V      | Stromversorgung |

#### E-Ink Display (über Waveshare Adapter) ➔ ESP32

| Display Pin | ESP32 Pin | Funktion |
|-------------|-----------|----------|
| CS          | GPIO 5    | Chip Select |
| DC          | GPIO 17   | Data/Command |
| RST         | GPIO 16   | Reset |
| BUSY        | GPIO 4    | Busy Signal |
| SCK         | GPIO 18   | SPI Clock (geteilt mit RFID) |
| MOSI        | GPIO 23   | SPI MOSI (geteilt mit RFID) |
| GND         | GND       | Ground |
| VCC         | 3.3V/5V   | Stromversorgung (je nach Adapter) |

### Schaltplan-Hinweise

⚠️ **Wichtig**:
- RFID und E-Ink Display **teilen sich den SPI-Bus** (SCK und MOSI)
- Jedes Gerät hat einen eigenen **Chip Select (CS)** Pin
- Der RFID Leser benötigt **3.3V** Stromversorgung
- Das E-Ink Display kann je nach Adapter **3.3V oder 5V** benötigen

## 💻 Software Installation

### 1. Voraussetzungen

- [Visual Studio Code](https://code.visualstudio.com/)
- [PlatformIO Extension](https://platformio.org/install/ide?install=vscode)

### 2. Projekt Setup

```bash
# Repository klonen
git clone https://github.com/yourusername/Countdowndisplay.git
cd Countdowndisplay

# In VS Code öffnen
code .
```

### 3. Bibliotheken

Alle benötigten Bibliotheken sind in der `platformio.ini` definiert und werden automatisch installiert:

- GxEPD2 (E-Ink Display Treiber)
- Adafruit GFX Library (Grafik)
- MFRC522 (RFID Reader)
- ArduinoJson (JSON Verarbeitung)
- ESP Async WebServer (Webserver)
- AsyncTCP (Async Kommunikation)

### 4. Hochladen

```bash
# Code kompilieren und hochladen
pio run -t upload

# Filesystem (Webinterface) hochladen
pio run -t uploadfs

# Serial Monitor öffnen
pio device monitor
```

## 🚀 Erste Schritte

### 1. Ersteinrichtung

Nach dem ersten Start erstellt der ESP32 einen WiFi Access Point:

- **SSID**: `CountdownDisplay`
- **Passwort**: `countdown123`

### 2. Webinterface öffnen

1. Mit dem WiFi verbinden
2. Browser öffnen und zu `http://192.168.4.1` navigieren
3. Das Webinterface wird angezeigt

### 3. WiFi konfigurieren (Optional)

Im Webinterface unter "WiFi Einstellungen":
1. SSID deines Netzwerks eingeben
2. Passwort eingeben
3. "Speichern & Neustarten" klicken
4. ESP32 verbindet sich mit deinem WiFi
5. Neue IP-Adresse im Serial Monitor ablesen

### 4. Countdown hinzufügen

1. Im Webinterface auf "Neu" klicken
2. RFID-Karte an den Leser halten
3. Auf "Karte Scannen" klicken
4. Name eingeben (z.B. "Laras Geburtstag")
5. Datum auswählen
6. "Speichern" klicken

### 5. Countdown anzeigen

Halte die konfigurierte RFID-Karte an den Leser - der Countdown wird automatisch auf dem E-Ink Display angezeigt!

## 📱 Webinterface Features

### System Status
- Zeigt aktuellen Modus (AP oder WiFi Client)
- IP-Adresse
- Verbundene SSID

### Countdown-Verwaltung
- ➕ Neue Countdowns hinzufügen
- ✏️ Bestehende Countdowns bearbeiten
- 🗑️ Countdowns löschen
- 👁️ Übersicht aller Countdowns mit verbleibenden Tagen
- ⏸️ Countdowns aktivieren/deaktivieren

### WiFi-Einstellungen
- Netzwerk konfigurieren
- Zwischen AP und Client Modus wechseln

### System
- System neu starten
- Alle Einstellungen persistent gespeichert

## 🔧 Anpassungen

### Display-Layout anpassen

Bearbeite `src/display.cpp` um das Layout zu ändern:

```cpp
void DisplayManager::showCountdown(const Countdown& countdown, int daysRemaining) {
    // Hier kannst du Fonts, Positionen und Text anpassen
}
```

### Pin-Konfiguration ändern

Bearbeite `include/config.h`:

```cpp
// RFID RC522 Pins
#define RFID_SS_PIN     21
#define RFID_RST_PIN    22
// ... weitere Pins
```

### Standard-WiFi Credentials

In `include/config.h`:

```cpp
#define WIFI_SSID       "CountdownDisplay"
#define WIFI_PASSWORD   "countdown123"
```

## 📊 Technische Details

### Speicherung

- **LittleFS** Filesystem für persistente Datenspeicherung
- **JSON Format** für Konfigurationsdateien
- Speicherort: `/config.json` im Flash-Speicher

### Zeit-Synchronisation

- **NTP** (Network Time Protocol) für automatische Zeitsynchronisation
- Server: `pool.ntp.org`, `time.nist.gov`
- Zeitzone: MEZ (GMT+1) mit automatischer Sommerzeit

### Display-Updates

- **Initialer Update**: Beim Erkennen einer neuen Karte
- **Periodischer Update**: Alle 60 Minuten (für Datumsänderung um Mitternacht)
- **Energieeffizient**: E-Ink benötigt nur beim Update Strom

### API Endpunkte

REST API für erweiterte Integration:

- `GET /api/countdowns` - Alle Countdowns abrufen
- `POST /api/countdowns` - Countdown hinzufügen
- `PUT /api/countdowns/:uid` - Countdown aktualisieren
- `DELETE /api/countdowns/:uid` - Countdown löschen
- `GET /api/wifi` - WiFi Einstellungen abrufen
- `POST /api/wifi` - WiFi Einstellungen setzen
- `GET /api/scan-card` - RFID Karte scannen
- `GET /api/status` - System Status
- `POST /api/restart` - System neu starten

## 🐛 Troubleshooting

### Display bleibt weiß

- Überprüfe die Pin-Verbindungen
- Stelle sicher, dass das Display mit Strom versorgt wird
- Prüfe im Serial Monitor auf Fehlermeldungen

### RFID Karte wird nicht erkannt

- Karte näher an den Leser halten
- Überprüfe SPI-Verbindungen (besonders SCK und MOSI)
- Stelle sicher, dass der Leser mit 3.3V versorgt wird
- Im Serial Monitor wird die UID angezeigt wenn eine Karte erkannt wird

### WiFi Verbindung schlägt fehl

- Überprüfe SSID und Passwort
- Stelle sicher, dass 2.4GHz WiFi verfügbar ist (ESP32 unterstützt kein 5GHz)
- Zurücksetzen auf AP-Modus: Datei `/config.json` löschen und neu starten

### Webinterface lädt nicht

- Stelle sicher, dass das Filesystem hochgeladen wurde: `pio run -t uploadfs`
- Überprüfe IP-Adresse im Serial Monitor
- Cache des Browsers leeren

### Zeit ist falsch

- Internet-Verbindung prüfen (für NTP)
- Zeitzone in `src/main.cpp` anpassen bei Bedarf
- NTP-Server können bis zu 30 Sekunden für Synchronisation benötigen

## 👨‍💻 Für Entwickler & AI-Assistenten

Wenn du mit diesem Projekt entwickeln möchtest oder ein AI-Assistent bist, der beim Entwickeln hilft, lies bitte die **[CLAUDE.md](CLAUDE.md)** Datei. Sie enthält:

- Detaillierte Architektur-Dokumentation
- Codebase-Struktur und Komponenten
- Entwicklungsrichtlinien und Best Practices
- API-Dokumentation
- Hardware-Konfiguration Details
- Build & Deploy Anleitung

## 📝 Lizenz

MIT License - siehe LICENSE Datei für Details

## 🤝 Beitragen

Contributions sind willkommen! Bitte:
1. Fork das Repository
2. Erstelle einen Feature Branch
3. Committe deine Änderungen
4. Push zum Branch
5. Erstelle einen Pull Request

## 👏 Credits

- E-Ink Display: [Waveshare](https://www.waveshare.com/)
- RFID Library: [MFRC522](https://github.com/miguelbalboa/rfid)
- E-Ink Library: [GxEPD2](https://github.com/ZinggJM/GxEPD2)

## 📞 Support

Bei Fragen oder Problemen:
- Erstelle ein [Issue](https://github.com/yourusername/Countdowndisplay/issues)
- Überprüfe die Serial Monitor Ausgaben für Debug-Informationen

---

**Viel Spaß mit deinem Countdown Display!** 🎉
