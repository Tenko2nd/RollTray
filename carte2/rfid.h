#ifndef RFID_H
#define RFID_H

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN  D10
#define RST_PIN D9

extern MFRC522 rfid;

extern byte knownUID0[];
extern byte knownUID1[];
extern byte knownUID2[];
extern byte knownUID3[];
extern byte knownUIDfin[];
extern byte UID_SIZE;

bool compareUID(byte uid1[], byte uid2[], byte size);
void printUID(byte uid[], byte size);
int read_card();

#endif