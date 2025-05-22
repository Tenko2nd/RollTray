#include <Arduino.h> 
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <WiFiClientSecure.h>

extern const char* currentFirmwareVersion_OTA;

/**
 * @brief Fonction principale pour gérer les mises à jour OTA
 */
void handleOTAUpdates();

