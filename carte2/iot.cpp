#include "iot.h" // This MUST be included to link declarations with definitions

// --- DEFINITIONS of ALL Global Variables and Objects ---

char mqtt_server[40] = "192.168.8.223";
char mqtt_port[6] = "1883";
const char* mqtt_card_topic = "RollTray/topic/cards";
const char* mqtt_button_topic = "RollTray/topic/button";
const char* mqtt_buzzer_topic = "RollTray/topic/buzzer";
const char* mqtt_confirm_card_topic = "RollTray/confirmation/cards";
const char* mqtt_confirm_btn_topic = "RollTray/confirmation/button";
const char* mqtt_confirm_buzzer_topic = "RollTray/confirmation/buzzer";
const char* clientID = "esp32Client-";
bool waitingForCardAck = false;
bool waitingForBtnAck = false;

unsigned long lastSendTime = 0;
unsigned long now = 0;
const long resendInterval = 1000;

int CardMsgId = 0;
int BtnMsgId = 0;
int cardCount = 0;
int btnCount = 0;

int value_buzzer = -1;

WiFiClient espClient;
PubSubClient client(espClient);

bool shouldSaveConfig = false;

// --- Function DEFINITIONS ---

void saveConfigCallback () {
  Serial.println("Callback: Connexion WiFi établie. Devrait sauvegarder la config MQTT.");
  shouldSaveConfig = true;
}

void setupWifiManager() {
  // WiFiManager
  WiFiManager wm;

  // Décommenter pour réinitialiser les paramètres WiFi sauvegardés (pour test)
  // wm.resetSettings();

  // Définir le callback pour sauvegarder la config MQTT après connexion
  wm.setSaveConfigCallback(saveConfigCallback);

  // Définir un timeout pour le portail de configuration (en secondes)
  // Si personne ne se connecte au portail dans ce délai, il continue (ou redémarre)
  wm.setConfigPortalTimeout(180); // 3 minutes

  // Ajouter des paramètres personnalisés pour MQTT
  WiFiManagerParameter custom_mqtt_server("server", "Serveur MQTT", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "Port MQTT", mqtt_port, 6);

  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_port);

  // Tente de se connecter au WiFi. S'il échoue, démarre le portail de configuration.
  // Le nom du point d'accès sera "AutoConnectAP" par défaut, ou celui que vous spécifiez.
  String apName = "ESP32-Nano-Config-" + String(WiFi.macAddress()); // Nom unique pour l'AP
  if (!wm.autoConnect(apName.c_str(), "password123")) { // "password123" est le mot de passe de l'AP de config
    Serial.println("Échec de la connexion et timeout du portail de config atteint.");
    Serial.println("Redémarrage...");
    delay(3000);
    ESP.restart(); // Redémarre si le portail a expiré sans connexion
  } else {
    Serial.println("Connecté au WiFi!");
    // Récupérer les valeurs des paramètres personnalisés
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());

    Serial.println("Paramètres MQTT récupérés du portail:");
    Serial.print("Serveur: "); Serial.println(mqtt_server);
    Serial.print("Port: "); Serial.println(mqtt_port);
  }
}

void callbackMqtt(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message reçu [");
  Serial.print(topic); 
  Serial.print("] : ");

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  if (String(topic) == mqtt_confirm_card_topic) {
    StaticJsonDocument<100> doc;
    DeserializationError error = deserializeJson(doc, message);
    if (!error) {
      int ackId = doc["id"];
      if (ackId == CardMsgId) {
        Serial.println("ACK correspond à l’ID envoyé, prêt à envoyer le prochain message.");
        waitingForCardAck = false;
        cardCount = 0;
      }else{
        Serial.println((String)"Erreur ID de confirmation. Get: " + ackId + ". Expected: " + CardMsgId);
      }
    } else {
      Serial.println("Erreur de parsing de l'ACK");
    }
  }else if (String(topic) == mqtt_confirm_btn_topic){
    StaticJsonDocument<100> doc;
    DeserializationError error = deserializeJson(doc, message);
    if (!error) {
      int ackId = doc["id"];
      if (ackId == BtnMsgId) {
        Serial.println("ACK correspond à l’ID envoyé, prêt à envoyer le prochain message.");
        waitingForBtnAck = false;
        btnCount = 0;
      }else{
        Serial.println((String)"Erreur ID de confirmation. Get: " + ackId + ". Expected: " + BtnMsgId);
      }
    } else {
      Serial.println("Erreur de parsing de l'ACK");
    }
  }

  if (String(topic) == mqtt_buzzer_topic) {
    StaticJsonDocument<100> doc;
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
      Serial.println("Erreur de parsing JSON");
      return;
    }

    int id_buzzer = doc["id_buzzer"];
    value_buzzer = doc["value_buzzer"];

    Serial.print("Message ID button : ");
    Serial.println(id_buzzer);
    Serial.print("Valeur bouton button: ");
    Serial.println(value_buzzer);

  // Envoi de la confirmation à l'autre code
    StaticJsonDocument<50> confirm_msg_buzzer;
    confirm_msg_buzzer["id"] = id_buzzer;
    char msgBuffer_buzzer[50];
    serializeJson(confirm_msg_buzzer, msgBuffer_buzzer);

    bool success_b = client.publish(mqtt_confirm_buzzer_topic, msgBuffer_buzzer);
    if (success_b) {
      Serial.print("Confirmation envoyée pour ID (buzzer) ");
      Serial.println(id_buzzer);
    } else {
      Serial.println("Échec d'envoi de la confirmation");
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
      client.subscribe(mqtt_confirm_card_topic);
      client.subscribe(mqtt_confirm_btn_topic);
      client.subscribe(mqtt_buzzer_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}