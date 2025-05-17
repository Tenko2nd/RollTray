#include <WiFi.h>          
#include <PubSubClient.h> 
#include <ArduinoJson.h>

#define SECRET_SSID "HelloMae" //moto g(10)_8536
#define SECRET_PASS "onepiece" //ok3647gt

// Constantes à envoyer à carte n°2 pour buzzer
extern const int JOYFUL_MUSIC;
extern const int NOT_HAPPY_MUSIC;
extern const int STOP_MUSIC;


// Configuration WiFi
extern const char* ssid;
extern const char* password;


// Configuration MQTT 
extern const char* mqtt_server;
extern const int mqtt_port;

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


extern bool waitingForBzrAck;
extern int buzzerCount;
extern int BzrMsgId;

extern int old_value_buzzer;
extern int current_value_buzzer;


void callback(char* topic, byte* payload, unsigned int length);
void setup_wifi();
void reconnect();
void send_message_buzzer_mqtt(int value);
void reinitialize();