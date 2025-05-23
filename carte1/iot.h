#include <WiFi.h>          
#include <PubSubClient.h> 
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include "base.h"

#include <FS.h>
#include <LittleFS.h>

// Constantes à envoyer à carte n°2 pour buzzer
extern const int CALIBRAGE_FAIL;
extern const int JOYFUL_MUSIC;
extern const int NOT_HAPPY_MUSIC;
extern const int STOP_MUSIC;


// Configuration WiFi
extern const char* ssid;
extern const char* password;


// Configuration MQTT 
extern char mqtt_server[40];
extern char mqtt_port_str[6];

extern const char* mqtt_card_topic;
extern const char* mqtt_button_topic;
extern const char* mqtt_buzzer_topic;

extern const char* mqtt_confirm_card_topic;
extern const char* mqtt_confirm_button_topic;
extern const char* mqtt_confirm_buzzer_topic;


extern const char* clientID;
extern WiFiClient espClient;
extern PubSubClient client; 

extern unsigned long lastSendTime;
extern unsigned long now;
extern const long resendInterval;

extern int value_card;
extern int value_button;

extern unsigned long scan_time;


extern bool waitingForBzrAck;
extern int buzzerCount;
extern int BzrMsgId;

extern int old_value_buzzer;
extern int current_value_buzzer;


// MQTT problème de connexion :
extern int mqtt_connection_attempts;
extern bool force_config_portal;
// Flag dans la section .noinit pour persister entre les ESP.restart()
extern uint8_t noinit_force_portal_flag __attribute__ ((section (".noinit")));
extern uint32_t noinit_first_run_check __attribute__ ((section (".noinit")));
extern const uint32_t NOINIT_FIRST_RUN_VALUE;


void setupWifiManager();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void send_message_buzzer_mqtt(int value);

// Fonction LITTLEFS
void loadMqttConfig();
void saveMqttConfig();

