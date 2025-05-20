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

// --- Déclaration des variable externes ---
// WIFI
extern const char* ssid;
extern const char* password;
extern const char* clientID;
extern WiFiClient espClient;
// MQTT
extern char mqtt_server[40];
extern char mqtt_port_str[6];
extern PubSubClient client;
// MQTT topics
extern const char* mqtt_card_topic;
extern const char* mqtt_button_topic;
extern const char* mqtt_buzzer_topic;
extern const char* mqtt_confirm_card_topic;
extern const char* mqtt_confirm_btn_topic;
extern const char* mqtt_confirm_buzzer_topic;
// MQTT QoS1
extern bool waitingForCardAck;
extern bool waitingForBtnAck;
extern int cardCount;
extern int btnCount;
// MQTT publish
extern unsigned long lastSendTime;
extern unsigned long now;
extern const long resendInterval;
extern int CardMsgId;
extern int BtnMsgId;
// MQTT receive
extern int value_buzzer;
// MQTT problemes de connection
extern int mqtt_connection_attempts;
extern bool force_config_portal;
// Flag dans la section .noinit pour persister entre les ESP.restart()
extern uint8_t noinit_force_portal_flag __attribute__ ((section (".noinit")));
extern uint32_t noinit_first_run_check __attribute__ ((section (".noinit")));
extern const uint32_t NOINIT_FIRST_RUN_VALUE;

// --- Prototypes ---
/**
 * @brief Charge la configuration MQTT depuis un fichier JSON sur LittleFS.
 */
void loadMqttConfig();
/**
 * @brief Sauvegarde la configuration MQTT actuelle dans un fichier JSON sur LittleFS.
 */
void saveMqttConfig();
/**
 * @brief Configure et démarre WiFiManager.
 * Permet à l'utilisateur de se connecter au Wi-Fi et d'entrer les paramètres MQTT via un portail
 */
void setupWifiManager();
/**
 * @brief Fonction de rappel lorsqu'un message est reçu
 * @param topic Le topic sur lequel le message a été reçu.
 * @param payload Le contenu du message
 * @param length La longueur du message.
 */
void callbackMqtt(char* topic, byte* payload, unsigned int length);
/**
 * @brief Tente de (re)connecter le client MQTT au broker.
 * Si la connexion échoue après un certain nombre de tentatives (`MAX_MQTT_ATTEMPTS`),
 * elle met le drapeau `force_config_portal` à vrai et redémarre l'ESP32
 * pour permettre à l'utilisateur de reconfigurer les paramètres.
 */
void reconnect();

#endif // IOT_H