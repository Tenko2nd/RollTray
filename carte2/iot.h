#ifndef IOT_H
#define IOT_H

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>      // Inclus par WiFiManager
#include <WebServer.h>      // Inclus par WiFiManager
#include <WiFiManager.h> 
#include <FS.h>            // Pour le système de fichiers
#include <LittleFS.h>      // Spécifiquement LittleFS
#include <PubSubClient.h>
#include <ArduinoJson.h>

// --- Extern DECLARATIONS for ALL shared global variables/objects ---
extern const char* ssid;
extern const char* password;
extern char mqtt_server[40];
extern char mqtt_port_str[6]; 
extern const char* mqtt_card_topic;
extern const char* mqtt_button_topic;
extern const char* mqtt_buzzer_topic;
extern const char* mqtt_confirm_card_topic;
extern const char* mqtt_confirm_btn_topic;
extern const char* mqtt_confirm_buzzer_topic;
extern const char* clientID;
extern bool waitingForCardAck;
extern bool waitingForBtnAck;
extern int cardCount;
extern int btnCount;


extern unsigned long lastSendTime;
extern unsigned long now;
extern const long resendInterval;

extern int CardMsgId;
extern int BtnMsgId;

extern int value_buzzer;

extern WiFiClient espClient;
extern PubSubClient client;

extern int mqtt_connection_attempts;
extern bool force_config_portal;

// --- Function DECLARATIONS (Prototypes) ---
void loadMqttConfig();
void saveMqttConfig();
void setupWifiManager();
void callbackMqtt(char* topic, byte* payload, unsigned int length);
void reconnect();
// Add any other function DECLARATIONS here

#endif // IOT_H