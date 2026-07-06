# Standard-Bilder für Countdown Display

## Zweck
Bilder in diesem Ordner werden **fest ins Filesystem eingebaut** und sind nach jedem `uploadfs` verfügbar.

## Verwendung

### 1. Bilder hierher kopieren
Kopiere deine BMP-Bilder in diesen Ordner:
- `geburtstag.bmp`
- `weihnachten.bmp`
- `ostern.bmp`
- etc.

### 2. Format beachten
**Wichtig:** Bilder müssen **1-bit monochrom BMP** sein!
- Empfohlene Größe: **250x250 Pixel**
- Format: 1-bit monochrom BMP

### 3. Mit GIMP konvertieren
1. Bild öffnen
2. Bild → Skalieren → 250x250 Pixel
3. Bild → Modus → Indiziert → 1-bit Palette
4. Datei → Exportieren als → BMP
5. BMP-Optionen: **1-bit** auswählen

### 4. Upload
Nach dem Hinzufügen von Bildern:
```bash
pio run -t uploadfs  # Filesystem mit Bildern hochladen
pio run -t upload    # Code hochladen
```

## Wichtig!

- **Nur bei Code-Änderungen:** `pio run -t upload` (Bilder bleiben erhalten)
- **Bei Webinterface-Änderungen:** `pio run -t uploadfs` + `upload` (überschreibt Runtime-Bilder!)

Runtime-Bilder (über Web hochgeladen) gehen bei `uploadfs` verloren!
