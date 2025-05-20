// OTA.h
#ifndef OTA_H
#define OTA_H

#include <Arduino.h> 
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <WiFiClientSecure.h>

extern const char* currentFirmwareVersion_OTA;

// Déclaration de la fonction principale pour vérifier et potentiellement lancer une OTA
// Cette fonction sera appelée depuis votre setup() ou loop() principal.
void handleOTAUpdates();

// Optionnel: Une fonction pour initialiser les aspects OTA si nécessaire
// (par exemple, si vous voulez séparer l'initialisation de la vérification)
// void setupOTA(); // Pour l'instant, on peut tout mettre dans handleOTAUpdates ou l'appeler depuis là

#endif // OTA_H