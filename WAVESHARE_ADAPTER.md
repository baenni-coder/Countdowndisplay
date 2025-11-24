# Waveshare e-Paper Adapter - Setup Guide

## Wichtige Informationen zum e-Paper Adapter

Der Waveshare e-Paper Adapter für ESP32 ist speziell für die Verbindung von e-Paper Displays mit ESP32 Boards entwickelt.

### Standard Pin-Belegung des Adapters

Der Waveshare e-Paper Adapter verwendet folgende Pins:

```
Display-Verbindung:
├─ BUSY  -> GPIO 4
├─ RST   -> GPIO 16
├─ DC    -> GPIO 17
├─ CS    -> GPIO 5
├─ CLK   -> GPIO 18
└─ DIN   -> GPIO 23
```

### Zusätzliche Komponenten am Adapter

1. **Power**: Der Adapter hat einen DC-DC Wandler für die Display-Spannung
2. **Anschlüsse**: FFC-Kabel Anschluss für das Display
3. **SD-Karte**: Optional SD-Karten Slot (wenn vorhanden)

### Verbindung mit RFID RC522

Da das Display und der RFID-Reader den SPI-Bus teilen, müssen folgende Punkte beachtet werden:

#### Geteilte Pins (SPI-Bus):
- **CLK/SCK** (GPIO 18) - Wird von beiden genutzt
- **DIN/MOSI** (GPIO 23) - Wird von beiden genutzt

#### Separate Pins für RFID:
- **SS/SDA** (GPIO 21) - RFID Chip Select
- **RST** (GPIO 22) - RFID Reset
- **MISO** (GPIO 19) - Nur für RFID (Display benötigt keinen MISO)

### Schaltplan

```
ESP32                     Waveshare Adapter           RFID RC522
=================================================================
GPIO 18 (SCK)  ─────┬──> CLK                    ┌──> SCK
                    └────────────────────────────┘

GPIO 23 (MOSI) ─────┬──> DIN                    ┌──> MOSI
                    └────────────────────────────┘

GPIO 19 (MISO) ────────────────────────────────────> MISO
GPIO 5         ──────> CS
GPIO 17        ──────> DC
GPIO 16        ──────> RST
GPIO 4         ──────> BUSY

GPIO 21        ────────────────────────────────────> SS/SDA
GPIO 22        ────────────────────────────────────> RST

GND            ─────┬──> GND                    ┌──> GND
                    └────────────────────────────┘

3.3V           ──────> VCC (am Adapter)    ┌──> 3.3V
                                          └────────

5V             ──────> 5V (falls benötigt)
```

## Häufige Probleme und Lösungen

### Display bleibt weiß oder zeigt nichts an

**Ursachen:**
1. SPI-Bus nicht korrekt initialisiert
2. Falsche Pin-Konfiguration
3. Lose Verbindung zum Display (FFC-Kabel)

**Lösungen:**
- Überprüfe, dass der SPI-Bus VOR Display und RFID initialisiert wird
- Stelle sicher, dass das FFC-Kabel korrekt eingesteckt ist
- Prüfe die Kontakte auf Beschädigungen
- Stelle sicher, dass die richtige Display-Version konfiguriert ist (GxEPD2_750_T7 für 7.5" V2)

### RFID-Karte wird nicht erkannt

**Ursachen:**
1. SPI-Bus Konflikt
2. Falsche SS-Pin Konfiguration
3. Zu große Entfernung zum Leser

**Lösungen:**
- Stelle sicher, dass RFID_SS_PIN (21) korrekt ist
- Halte die Karte näher an den Leser (< 2cm)
- Prüfe die 3.3V Versorgung des RFID-Readers

### Display aktualisiert sich sehr langsam

**Normal!** E-Ink Displays benötigen mehrere Sekunden für einen vollständigen Refresh. Das ist eine Eigenschaft der Technologie und kein Fehler.

Typische Refresh-Zeiten:
- 7.5" Display: 15-30 Sekunden
- Partial Update: 1-2 Sekunden (wenn unterstützt)

### Beide Geräte funktionieren einzeln, aber nicht zusammen

**Ursache:** SPI-Bus Konflikt

**Lösung:**
- Der SPI-Bus muss zentral in `main.cpp` initialisiert werden
- Jedes Gerät benötigt seinen eigenen CS (Chip Select) Pin
- RFID: GPIO 21 (SS)
- Display: GPIO 5 (CS)

## Waveshare Display Versionen

Wichtig: Es gibt verschiedene Versionen des 7.5" Displays!

### 7.5" Version 1 (alt)
- Driver: IL3897
- GxEPD2 Klasse: `GxEPD2_750`

### 7.5" Version 2 (aktuell)
- Driver: IL0371
- GxEPD2 Klasse: `GxEPD2_750_T7` ✅ **Wir verwenden diese!**

### 7.5" Version 3 (B/W/R)
- Driver: UC8179
- GxEPD2 Klasse: `GxEPD2_750_T7_B74` (3-Farben)

### Wie erkenne ich meine Version?

1. Auf der Rückseite des Displays ist die Versionsnummer aufgedruckt
2. Das Produktdatenblatt gibt Auskunft
3. Test: Wenn das Display nicht funktioniert, könnte die falsche Version konfiguriert sein

## Code-Anpassungen für verschiedene Displays

Wenn du ein anderes Display verwendest, ändere in `include/display.h`:

```cpp
// Für 7.5" V2 (aktuell):
GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT>* display;

// Für 7.5" V1:
GxEPD2_BW<GxEPD2_750, GxEPD2_750::HEIGHT>* display;

// Für 4.2" Display:
GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT>* display;
```

Und in `src/display.cpp` im Konstruktor entsprechend anpassen.

## Spannungsversorgung

**Wichtig für die Stromversorgung:**

- ESP32: Benötigt 5V über USB oder VIN (wird intern zu 3.3V)
- RFID RC522: Benötigt 3.3V (NIEMALS 5V!)
- E-Paper Display: Der Adapter wandelt die Spannung für das Display um

**Empfohlene Stromversorgung:**
- USB-Netzteil mit mindestens 1A
- Beim E-Ink Update kann der Strombedarf kurzzeitig höher sein

## Test-Reihenfolge

Beim ersten Testen des Systems:

1. **Nur ESP32**: Programmiere den ESP32 und prüfe Serial Monitor
2. **ESP32 + Display**: Teste erst das Display allein
3. **ESP32 + RFID**: Teste dann den RFID-Reader allein
4. **Alles zusammen**: Kombiniere beide Komponenten

So kannst du Probleme leichter eingrenzen!

## Weitere Ressourcen

- [Waveshare Wiki](https://www.waveshare.com/wiki/Main_Page)
- [GxEPD2 Bibliothek](https://github.com/ZinggJM/GxEPD2)
- [MFRC522 Bibliothek](https://github.com/miguelbalboa/rfid)
