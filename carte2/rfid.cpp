#include "rfid.h"

extern MFRC522 rfid(SS_PIN, RST_PIN);

byte knownUID0[] = {0x4C, 0xEB, 0x14, 0x05}; //  UID carte 0
byte knownUID1[] = {0x94, 0x1D, 0x71, 0x28}; //  UID carte 1
byte knownUID2[] = {0xB4, 0x82, 0x15, 0x28}; //  UID carte 2
byte knownUID3[] = {0x94, 0x67, 0x8F, 0x28}; //  UID carte 3
byte UID_SIZE = 4;

/**
 * Fonction pour comparer deux UID.
 * Prend en compte la taille des UID pour éviter les erreurs.
 * @param uid1 Premier UID (tableau d'octets)
 * @param uid2 Deuxième UID (tableau d'octets)
 * @param size Taille réelle de l'UID lu (fournie par rfid.uid.size)
 * @return true si les UID sont identiques ET ont la taille attendue (UID_SIZE), false sinon.
 */
bool compareUID(byte uid1[], byte uid2[], byte size) {
  if (size != UID_SIZE) {
    return false;
  }

  // Compare chaque octet des deux UID
  for (byte i = 0; i < UID_SIZE; i++) {
    if (uid1[i] != uid2[i]) {
      return false;
    }
  }
  // Si on arrive ici, tous les octets sont identiques et la taille correspond
  return true;
}

/**
 * Fonction utilitaire pour afficher un UID sur le port série en HEX.
 * @param uid L'UID à afficher (tableau d'octets)
 * @param size La taille de l'UID
 */
void printUID(byte uid[], byte size) {
  for (byte i = 0; i < size; i++) {
    Serial.print(uid[i] < 0x10 ? " 0" : " "); // Ajoute un 0 devant si l'octet est < 16 (0x10)
    Serial.print(uid[i], HEX);
  }
}

int read_card(){
  // Cherche une nouvelle carte
  if (rfid.PICC_IsNewCardPresent()) {
    // Tente de lire l'UID de la carte
    if (rfid.PICC_ReadCardSerial()) {
      
      // Affiche l'UID de la carte lue (pour le débogage)
      // Serial.print("UID lu: ");
      // printUID(rfid.uid.uidByte, rfid.uid.size);
      // Serial.println();

      // Compare l'UID lu avec les UID connus
      if (compareUID(rfid.uid.uidByte, knownUID0, rfid.uid.size)) { // Base
        return 0;

      } else if (compareUID(rfid.uid.uidByte, knownUID1, rfid.uid.size)) { // Carte 1
        return 1;

      } else if (compareUID(rfid.uid.uidByte, knownUID2, rfid.uid.size)) { // Carte 2
        return 2;

      } else if (compareUID(rfid.uid.uidByte, knownUID3, rfid.uid.size)) { // Carte 3
        return 3;

      } else {
        return -1;
      }

      Serial.println("-----------------------------------------");

      // Arrête la communication avec la carte actuelle
      rfid.PICC_HaltA();
      // Arrête le chiffrement (si actif)
      rfid.PCD_StopCrypto1();
    }
  }
  return -1;
}