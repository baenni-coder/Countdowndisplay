# Display-Version testen

Falls dein Display eine andere Version ist, ändere diese Zeilen:

## In `include/display.h` Zeile 28:

### Für 7.5" Version 2 (aktuell konfiguriert):
```cpp
GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT>* display;
```

### Für 7.5" Version 1:
```cpp
GxEPD2_BW<GxEPD2_750, GxEPD2_750::HEIGHT>* display;
```

### Für 7.5" Version 3:
```cpp
GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT>* display;  // Gleich wie V2
```

## In `src/display.cpp` Zeile 8-10:

### Für 7.5" Version 2 (aktuell):
```cpp
display = new GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT>(
    GxEPD2_750_T7(EPD_CS_PIN, EPD_DC_PIN, EPD_RST_PIN, EPD_BUSY_PIN)
);
```

### Für 7.5" Version 1:
```cpp
display = new GxEPD2_BW<GxEPD2_750, GxEPD2_750::HEIGHT>(
    GxEPD2_750(EPD_CS_PIN, EPD_DC_PIN, EPD_RST_PIN, EPD_BUSY_PIN)
);
```

## Display-Version auf der Rückseite ablesen

Die Version steht normalerweise auf einem Aufkleber:
- `7.5inch e-Paper (B) V2` → Version 2 (GxEPD2_750_T7)
- `7.5inch e-Paper V1` → Version 1 (GxEPD2_750)
- `GDEW075T7` → Version 2 (GxEPD2_750_T7)

## Test-Methode

Wenn du unsicher bist, welche Version du hast:

1. Versuche Version 1 (GxEPD2_750)
2. Kompiliere und lade hoch
3. Wenn es immer noch "Busy Timeout" gibt, versuche andere Versionen
