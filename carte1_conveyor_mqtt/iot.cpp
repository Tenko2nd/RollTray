#include "iot.h"


// --- Configuration WiFi ---
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;

// --- Configuration MQTT ---
const char* mqtt_server = "192.168.193.23";
const int mqtt_port = 1883;
const char* mqtt_card_topic = "RollTray/topic/cards";
const char* mqtt_button_topic = "RollTray/topic/button";
const char* mqtt_confirm_card_topic = "RollTray/confirmation/cards";
const char* mqtt_confirm_button_topic = "RollTray/confirmation/button";

const char* clientID = "esp32SubClient-";

int value_card = -1;
int value_button = -1;

WiFiClient espClient;
PubSubClient client(espClient);


// Callback exécutée quand un message arrive
void callback(char* topic, byte* payload, unsigned int length) {

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  if (String(topic) == mqtt_card_topic) {
    StaticJsonDocument<100> doc;
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
      Serial.println("Erreur de parsing JSON");
      return;
    }

    int id_card = doc["id_card"];
    value_card = doc["value_card"];

    Serial.print("Message ID card : ");
    Serial.println(id_card);
    Serial.print("Valeur card : ");
    Serial.println(value_card);


    // Envoi de la confirmation à l'autre code
    StaticJsonDocument<50> confirm_msg;
    confirm_msg["id"] = id_card;
    char msgBuffer[50];
    serializeJson(confirm_msg, msgBuffer);

    bool success = client.publish(mqtt_confirm_card_topic, msgBuffer);
    if (success) {
      Serial.print("Confirmation envoyée pour ID (card)");
      Serial.println(id_card);
    } else {
      Serial.println("Échec d'envoi de la confirmation");
    }
  }

  if (String(topic) == mqtt_button_topic) {
    StaticJsonDocument<100> doc;
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
      Serial.println("Erreur de parsing JSON");
      return;
    }

    int id_button = doc["id_button"];
    value_button = doc["value_button"];

    Serial.print("Message ID button : ");
    Serial.println(id_button);
    Serial.print("Valeur bouton button: ");
    Serial.println(value_button);


    // Envoi de la confirmation à l'autre code
    StaticJsonDocument<50> confirm_msg_button;
    confirm_msg_button["id"] = id_button;
    char msgBuffer_button[50];
    serializeJson(confirm_msg_button, msgBuffer_button);

    bool success_b = client.publish(mqtt_confirm_button_topic, msgBuffer_button);
    if (success_b) {
      Serial.print("Confirmation envoyée pour ID (bouton) ");
      Serial.println(id_button);
    } else {
      Serial.println("Échec d'envoi de la confirmation");
    }
  }

  Serial.println("----------");
}


void setup_wifi() {
  delay(10);
  Serial.println("Connexion wifi");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("\nConnecté. Adresse IP : ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentative de connexion MQTT...");
    String clientIdString = clientID;
    clientIdString += String(random(0xffff), HEX);

    if (client.connect(clientIdString.c_str())) {
      Serial.println("connecté");

      // S'abonner au topic des cartes
      if (client.subscribe(mqtt_card_topic)) {
        Serial.print("Abonné à : ");
        Serial.println(mqtt_card_topic);
      } else {
        Serial.print("Échec abonnement à ");
        Serial.println(mqtt_card_topic);
      }

      // S'abonner au topic des boutons
      if (client.subscribe(mqtt_button_topic)) {
        Serial.print("Abonné à : ");
        Serial.println(mqtt_button_topic);
      } else {
        Serial.print("Échec abonnement à ");
        Serial.println(mqtt_button_topic);
      }

    } else {
      Serial.print("Échec, rc=");
      Serial.print(client.state());
      Serial.println(" nouvelle tentative dans 5 secondes");
      delay(5000);
    }
  }
}