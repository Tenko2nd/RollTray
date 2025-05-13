#ifndef IOT_H
#define IOT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h" // Assumed to contain #defines like: #define SECRET_SSID "mySSID"

// --- Extern DECLARATIONS for ALL shared global variables/objects ---
extern const char* ssid;
extern const char* password;
extern const char* mqtt_server;
extern const int mqtt_port; // Even for const int, extern is safest for cross-file sharing
extern const char* mqtt_publish_topic;
extern const char* mqtt_confirm_topic;
extern const char* clientID;
extern bool waitingForAck;

extern unsigned long lastSendTime;
extern unsigned long now;
extern const long resendInterval;

extern int messageId;

extern WiFiClient espClient;
extern PubSubClient client;

// --- Function DECLARATIONS (Prototypes) ---
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
// Add any other function DECLARATIONS here

#endif // IOT_H