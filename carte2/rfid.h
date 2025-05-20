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

/**
 * @brief Compare deux UID.
 * Prend en compte la taille des UID pour éviter les erreurs.
 * @param uid1 Premier UID
 * @param uid2 Deuxième UID
 * @param size Taille réelle de l'UID lu
 * @return true si les UID sont identiques ET ont la taille attendue, false sinon.
 */
bool compareUID(byte uid1[], byte uid2[], byte size);

/**
 * @brief Afficher un UID
 * @param uid L'UID à afficher
 * @param size La taille de l'UID
 */
void printUID(byte uid[], byte size);

/**
* @brief Lit si une carte RFID est présent devant le capteur
* @return Le numero de la carte si c'est une carte connue, -1 sinon
*/
int read_card();

#endif