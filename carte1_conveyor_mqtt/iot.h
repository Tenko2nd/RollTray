#include <WiFi.h>          
#include <PubSubClient.h> 
#include <ArduinoJson.h>

#define SECRET_SSID "moto g(10)_8536"
#define SECRET_PASS "ok3647gt"

// --- Configuration WiFi ---
extern const char* ssid;
extern const char* password;

// --- Configuration MQTT ---
extern const char* mqtt_server;
extern const int mqtt_port;

extern const char* mqtt_card_topic;
extern const char* mqtt_button_topic;
extern const char* mqtt_confirm_card_topic;
extern const char* mqtt_confirm_button_topic;
extern const char* clientID;
extern int value_card;
extern int value_button;

extern WiFiClient espClient;
extern PubSubClient client; 


void callback(char* topic, byte* payload, unsigned int length);
void setup_wifi();
void reconnect();