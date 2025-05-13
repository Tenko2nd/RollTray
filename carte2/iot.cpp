#include "iot.h" // This MUST be included to link declarations with definitions

// --- DEFINITIONS of ALL Global Variables and Objects ---

const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS; 
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_publish_topic = "RollTray/topic";
const char* mqtt_confirm_topic = "RollTray/confirmation";
const char* clientID = "esp32Client-";
bool waitingForAck = false;

unsigned long lastSendTime = 0;
unsigned long now = 0;
const long resendInterval = 5000;

int messageId = 0;

WiFiClient espClient;
PubSubClient client(espClient);

// --- Function DEFINITIONS ---

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message reçu [");
  Serial.print(topic); 
  Serial.print("] : ");

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  if (String(topic) == mqtt_confirm_topic) {
    StaticJsonDocument<100> doc;
    DeserializationError error = deserializeJson(doc, message);
    if (!error) {
      int ackId = doc["id"];
      if (ackId == messageId) {
        Serial.println("ACK correspond à l’ID envoyé, prêt à envoyer le prochain message.");
        waitingForAck = false;
      }else{
        Serial.println((String)"Erreur ID de confirmation. Get: " + ackId + ". Expected: " + messageId);
      }
    } else {
      Serial.println("Erreur de parsing de l'ACK");
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientIdString = clientID;
    clientIdString += String(random(0xffff), HEX);

    if (client.connect(clientIdString.c_str())) {
      Serial.println("connected");
      client.subscribe(mqtt_confirm_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}