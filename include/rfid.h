#ifndef RFID_H
#define RFID_H

#include <Arduino.h>
#include <MFRC522.h>
#include <SPI.h>

class RFIDReader {
public:
    RFIDReader();
    bool begin();
    bool cardPresent();
    String readCardUID();
    String getLastCardUID() const { return lastUID; }  // Letzten UID abrufen
    unsigned long getLastReadTime() const { return lastReadTime; }  // Letzten Lesezeitpunkt

private:
    MFRC522 mfrc522;
    String lastUID;
    unsigned long lastReadTime;

    String uidToString(byte* uid, byte size);
};

extern RFIDReader rfidReader;

#endif
