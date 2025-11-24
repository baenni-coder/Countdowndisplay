# Troubleshooting Guide

Dieser Guide hilft bei der Lösung häufiger Probleme.

## Kompilierungsfehler

### "ESPAsyncWebServer.h: No such file or directory"

**Ursache:** Bibliotheken noch nicht installiert

**Lösung:**
```bash
# In PlatformIO:
pio lib install

# Oder:
pio run
```

Die Bibliotheken werden beim ersten Build automatisch heruntergeladen. Das kann einige Minuten dauern.

---

### "Multiple libraries were found for..."

**Ursache:** Mehrere Versionen einer Bibliothek installiert

**Lösung:**
```bash
# Lösche Build-Cache:
pio run -t clean

# Lösche .pio Ordner:
rm -rf .pio

# Neu kompilieren:
pio run
```

---

### "error: 'JsonDocument' was not declared in this scope"

**Ursache:** Falsche ArduinoJson Version

**Lösung:**
Stelle sicher, dass in `platformio.ini` Version 6 verwendet wird:
```ini
bblanchon/ArduinoJson@^6.21.3
```

NICHT Version 7!

---

### Kompilierung hängt bei "Linking..."

**Ursache:** Zu wenig RAM oder langsame Festplatte

**Lösung:**
- Warte geduldig (kann bis zu 5 Minuten dauern)
- Schließe andere Programme
- Wenn es länger als 10 Minuten dauert: Prozess abbrechen und neu starten

---

## Upload-Fehler

### "A fatal error occurred: Failed to connect to ESP32"

**Ursachen:**
1. Falscher COM-Port
2. ESP32 nicht im Boot-Modus
3. Treiber fehlen

**Lösungen:**

**1. COM-Port prüfen:**
```bash
# In PlatformIO Terminal:
pio device list
```

**2. ESP32 manuell in Boot-Modus versetzen:**
- BOOT-Taste gedrückt halten
- EN-Taste kurz drücken
- BOOT-Taste loslassen
- Upload erneut versuchen

**3. Treiber installieren:**
- CP210x Treiber: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
- CH340 Treiber: http://www.wch-ic.com/downloads/CH341SER_ZIP.html

---

### "Timed out waiting for packet header"

**Ursache:** Kommunikationsproblem mit ESP32

**Lösungen:**
1. USB-Kabel wechseln (viele Kabel können keine Daten übertragen!)
2. Anderen USB-Port verwenden
3. ESP32 zurücksetzen (EN-Taste)
4. Upload-Geschwindigkeit reduzieren in `platformio.ini`:
```ini
upload_speed = 115200
```

---

### "No module named 'serial'"

**Ursache:** Python-Paket `pyserial` fehlt

**Lösung:**
```bash
pip install pyserial
```

---

## Laufzeit-Fehler

### ESP32 startet nicht / bootet ständig neu

**Symptome im Serial Monitor:**
```
Brownout detector was triggered
rst:0x10 (RTCWDT_RTC_RESET)
```

**Ursache:** Zu wenig Strom

**Lösungen:**
1. Verwende ein stärkeres USB-Netzteil (mindestens 1A)
2. Schließe nur USB-Kabel an, kein Display/RFID während des Tests
3. Prüfe USB-Kabel (manche haben zu hohen Widerstand)

---

### "FEHLER: Storage konnte nicht initialisiert werden!"

**Ursache:** LittleFS nicht formatiert oder defekt

**Lösung:**
```bash
# Filesystem formatieren und neu hochladen:
pio run -t erase
pio run -t uploadfs
pio run -t upload
```

⚠️ **Warnung:** `erase` löscht ALLE Daten auf dem ESP32!

---

### "FEHLER: RFID Reader konnte nicht initialisiert werden!"

**Mögliche Ursachen:**
1. Falsche Verkabelung
2. RFID-Modul defekt
3. 5V statt 3.3V (RFID-Modul zerstört!)

**Lösungen:**
1. **Verkabelung prüfen:**
   - SS -> GPIO 21
   - SCK -> GPIO 18
   - MOSI -> GPIO 23
   - MISO -> GPIO 19
   - RST -> GPIO 22
   - 3.3V -> 3.3V (NICHT 5V!)
   - GND -> GND

2. **RFID-Modul testen:**
   ```bash
   # Serial Monitor öffnen:
   pio device monitor

   # Achte auf Ausgabe:
   # "RFID RC522 gefunden, Version: 0x92" oder ähnlich
   ```

3. **Wenn Version 0x00 oder 0xFF:**
   - Modul ist defekt oder nicht verbunden
   - Prüfe Lötstellen
   - Versuche ein anderes Modul

---

### "FEHLER: Display konnte nicht initialisiert werden!"

**Mögliche Ursachen:**
1. Display nicht angeschlossen
2. FFC-Kabel lose
3. Falsche Display-Version im Code

**Lösungen:**

1. **Hardware prüfen:**
   - FFC-Kabel korrekt eingesteckt?
   - Verriegelung geschlossen?
   - Pins am Waveshare Adapter korrekt?

2. **Display-Version prüfen:**

   Schaue auf der Rückseite des Displays nach der Version.

   In `include/display.h` anpassen:
   ```cpp
   // Für 7.5" V2 (Standard):
   GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT>* display;

   // Für 7.5" V1:
   GxEPD2_BW<GxEPD2_750, GxEPD2_750::HEIGHT>* display;
   ```

3. **SPI-Bus prüfen:**

   Stelle sicher, dass SPI in `main.cpp` initialisiert wird:
   ```cpp
   SPI.begin(EPD_SCK_PIN, RFID_MISO_PIN, EPD_MOSI_PIN, -1);
   ```

---

### Display zeigt nur teilweise oder verfälschte Inhalte

**Ursachen:**
1. Lose Verbindung
2. Stromversorgung instabil
3. SPI-Bus Probleme

**Lösungen:**
1. FFC-Kabel neu einstecken
2. Stärkeres Netzteil verwenden
3. Kürzere Kabel zwischen ESP32 und Adapter

---

### WiFi verbindet nicht

**Symptom:**
```
Verbinde zu WiFi: MeinNetzwerk
....................
WiFi Verbindung fehlgeschlagen
```

**Lösungen:**

1. **2.4 GHz prüfen:**
   ESP32 unterstützt NUR 2.4 GHz WiFi, NICHT 5 GHz!

2. **SSID/Passwort prüfen:**
   - Groß-/Kleinschreibung beachten
   - Sonderzeichen können Probleme verursachen
   - Teste mit einem Hotspot vom Handy

3. **WiFi-Land-Code setzen:**
   ```cpp
   // In main.cpp vor WiFi.begin():
   WiFi.setTxPower(WIFI_POWER_19_5dBm);
   ```

4. **Auf Access Point zurückfallen:**
   ```bash
   # Lösche Config-Datei über Serial Monitor:
   # Nach dem Neustart startet der AP wieder
   ```

---

### Webinterface lädt nicht

**Mögliche Ursachen:**
1. Filesystem nicht hochgeladen
2. Falsche IP-Adresse
3. Browser-Cache

**Lösungen:**

1. **Filesystem hochladen:**
   ```bash
   pio run -t uploadfs
   ```

2. **IP-Adresse prüfen:**
   ```bash
   # Serial Monitor öffnen:
   pio device monitor

   # Suche nach:
   # "Webinterface: http://192.168.x.x"
   ```

3. **Browser-Cache leeren:**
   - Ctrl+Shift+R (Windows/Linux)
   - Cmd+Shift+R (Mac)
   - Oder inkognito Modus verwenden

---

### Zeit ist falsch / NTP funktioniert nicht

**Symptom:**
```
WARNUNG: NTP Zeit-Synchronisation fehlgeschlagen!
```

**Ursachen:**
1. Keine Internet-Verbindung
2. Firewall blockiert NTP
3. NTP-Server nicht erreichbar

**Lösungen:**

1. **Internet-Verbindung prüfen:**
   - Kann der ESP32 ins Internet?
   - Ping Google.com zum Test

2. **Alternative NTP-Server:**
   ```cpp
   // In main.cpp ändern:
   configTime(3600, 3600, "de.pool.ntp.org", "europe.pool.ntp.org");
   ```

3. **Zeitzone anpassen:**
   ```cpp
   // MEZ = GMT+1, MESZ = GMT+2
   configTime(3600, 3600, "pool.ntp.org");
   ```

---

## Performance-Probleme

### Display-Update dauert sehr lange (>1 Minute)

**Normal für E-Ink!** Aber wenn es EXTREM lange dauert:

**Lösungen:**
1. SPI-Geschwindigkeit erhöhen (experimentell):
   ```cpp
   // In display.cpp nach display->init():
   SPI.setFrequency(4000000); // 4 MHz
   ```

2. Partial Updates verwenden (wenn Display unterstützt):
   ```cpp
   display->setPartialWindow(x, y, w, h);
   // ... Zeichnen ...
   display->nextPage();
   ```

---

### RFID-Karten werden verzögert erkannt

**Normal!** Das System prüft nur einmal pro Sekunde.

Wenn du das ändern möchtest:
```cpp
// In main.cpp:
const unsigned long CARD_CHECK_INTERVAL = 500; // 500ms statt 1000ms
```

⚠️ Beachte: Häufigeres Prüfen erhöht den Stromverbrauch.

---

## Debug-Tipps

### Serial Monitor verwenden

```bash
# Monitor öffnen:
pio device monitor

# Monitor mit Filter:
pio device monitor --filter colorize

# Monitor beenden:
Ctrl+C
```

### Debug-Ausgaben aktivieren

In `platformio.ini`:
```ini
build_flags =
    -DCORE_DEBUG_LEVEL=5  ; Maximum Debug-Level
```

### Reset durchführen

```bash
# Software-Reset über API:
curl -X POST http://ESP_IP/api/restart

# Hardware-Reset:
EN-Taste am ESP32 drücken
```

---

## Weitere Hilfe

Wenn diese Lösungen nicht helfen:

1. **Serial Monitor Log kopieren** und zur Analyse bereitstellen
2. **Hardware-Aufbau fotografieren**
3. **PlatformIO Verbose Output**:
   ```bash
   pio run -v
   ```

4. **GitHub Issues** prüfen:
   - GxEPD2: https://github.com/ZinggJM/GxEPD2/issues
   - MFRC522: https://github.com/miguelbalboa/rfid/issues
