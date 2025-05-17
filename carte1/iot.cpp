#include "iot.h"

extern const int JOYFUL_MUSIC = 2;
extern const int NOT_HAPPY_MUSIC = 1;
extern const int STOP_MUSIC = 0;

const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;

const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_card_topic = "RollTray/topic/cards";
const char* mqtt_button_topic = "RollTray/topic/button";
const char* mqtt_confirm_card_topic = "RollTray/confirmation/cards";
const char* mqtt_confirm_button_topic = "RollTray/confirmation/button";

const char* mqtt_buzzer_topic = "RollTray/topic/buzzer";
const char* mqtt_confirm_buzzer_topic = "RollTray/confirmation/buzzer";

const char* clientID = "esp32SubClient-";

unsigned long lastSendTime = 0;
unsigned long now = 0;
const long resendInterval = 1000;

bool waitingForBzrAck = false;
int buzzerCount = 0;
int BzrMsgId = 0;

int value_card = -1;
int value_button = -1;
int old_value_buzzer = -1;
int current_value_buzzer = -1;

WiFiClient espClient;
PubSubClient client(espClient);


// Callback exécutée quand un message arrive
void callback(char* topic, byte* payload, unsigned int length) {

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Réception du message pour la carte + confirmation
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

    // -> Confirmation
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

  // Réception du message pour le BP + confirmation
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


    // -> Confirmation
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

  // Réception de la confirmation pour le buzzer
  if (String(topic) == mqtt_confirm_buzzer_topic) {
    StaticJsonDocument<50> doc;
    DeserializationError error = deserializeJson(doc, message);
    if (!error) {
      int ackId_buzzer = doc["id"];
      if (ackId_buzzer == BzrMsgId) {
        Serial.println("ACK correspond à l’ID envoyé (buzzer), prêt à envoyer le prochain message.");
        waitingForBzrAck = false;
        buzzerCount = 0;
      } else {
        Serial.println((String) "Erreur ID de confirmation. Get: " + ackId_buzzer + ". Expected: " + BzrMsgId);
      }
    } else {
      Serial.println("Erreur de parsing de l'ACK");
    }

    Serial.println("----------");
  }
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

// (Re)connexion aux topics MQTT
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

      if (client.subscribe(mqtt_confirm_buzzer_topic)) {
        Serial.print("Abonné à : ");
        Serial.println(mqtt_confirm_buzzer_topic);
      } else {
        Serial.print("Échec abonnement à ");
        Serial.println(mqtt_confirm_buzzer_topic);
      }


    } else {
      Serial.print("Échec, rc=");
      Serial.print(client.state());
      Serial.println(" nouvelle tentative dans 5 secondes");
      delay(5000);
    }
  }
}


// Envoi du message au buzzer via MQTT
void send_message_buzzer_mqtt(int value) {

  StaticJsonDocument<100> doc;
  doc["id_buzzer"] = BzrMsgId;
  doc["value_buzzer"] = value;
  char msgBuffer[100];
  serializeJson(doc, msgBuffer);
  bool success = client.publish(mqtt_buzzer_topic, msgBuffer);
  if (success) {
    Serial.println("Message buzzer publié avec succes");
  } else {
    Serial.println("Échec de publication buzzer");
  }
  lastSendTime = now;
  buzzerCount++;
}

// Réinitialise toutes les variables définies
void reinitialize() {
  value_card = -1;
  value_button = -1;
  waitingForBzrAck = false;
  buzzerCount = 0;
  BzrMsgId = 0;
  old_value_buzzer = -1;
  current_value_buzzer = -1;
}