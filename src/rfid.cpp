#include "rfid.h"
#include "config.h"

RFIDReader rfidReader;

RFIDReader::RFIDReader() : mfrc522(RFID_SS_PIN, RFID_RST_PIN), lastReadTime(0) {
}

bool RFIDReader::begin() {
    // SPI Bus ist bereits in main.cpp initialisiert
    // Initialisiere nur das RFID Modul
    mfrc522.PCD_Init();
    delay(100);

    // Prüfe ob RFID Modul antwortet
    byte version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
    if (version == 0x00 || version == 0xFF) {
        Serial.println("RFID Modul nicht gefunden!");
        return false;
    }

    Serial.print("RFID RC522 gefunden, Version: 0x");
    Serial.println(version, HEX);
    return true;
}

bool RFIDReader::cardPresent() {
    // Prüfe ob eine neue Karte vorhanden ist
    if (!mfrc522.PICC_IsNewCardPresent()) {
        return false;
    }

    // Versuche die Karte zu lesen
    if (!mfrc522.PICC_ReadCardSerial()) {
        return false;
    }

    return true;
}

String RFIDReader::readCardUID() {
    if (!cardPresent()) {
        return "";
    }

    String uid = uidToString(mfrc522.uid.uidByte, mfrc522.uid.size);

    // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();

    lastUID = uid;
    lastReadTime = millis();

    Serial.print("Karte gelesen: ");
    Serial.println(uid);

    return uid;
}

String RFIDReader::uidToString(byte* uid, byte size) {
    String uidString = "";
    for (byte i = 0; i < size; i++) {
        if (uid[i] < 0x10) {
            uidString += "0";
        }
        uidString += String(uid[i], HEX);
    }
    uidString.toUpperCase();
    return uidString;
}
