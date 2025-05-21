#include "iot.h" // This MUST be included to link declarations with definitions

// --- DEFINITIONS DES VARIABLES ---
// MQTT
char mqtt_server[40] = "";
char mqtt_port_str[6] = "";
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

// Variables internes
bool shouldSaveConfig = false; // Sauvegarde configuration WIFI / MQTT
const char *mqttConfigFilePath = "/mqtt_config.json"; // Fichier littleFS Configuration MQTT

int mqtt_connection_attempts = 0;
const int MAX_MQTT_ATTEMPTS = 5; // Nombre maximal d'essais avant de forcer le portail
bool force_config_portal = false;
// Définition des variables .noinit
uint8_t noinit_force_portal_flag;
uint32_t noinit_first_run_check;
const uint32_t NOINIT_FIRST_RUN_VALUE = 0xCAFEBABE;

// --- Function DEFINITIONS ---
// Fonction pour charger la configuration MQTT depuis LittleFS
void loadMqttConfig() {
  if (!LittleFS.begin(true)) {
    Serial.println("Erreur montage LittleFS pour chargement config.");
    return;
  }
  Serial.println("LittleFS monté.");

  if (LittleFS.exists(mqttConfigFilePath)) {
    Serial.print("Lecture du fichier de configuration MQTT: ");
    Serial.println(mqttConfigFilePath);
    File configFile = LittleFS.open(mqttConfigFilePath, "r");
    if (configFile) {
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, configFile);
      if (error) {
        Serial.print(F("deserializeJson() a échoué: "));
        Serial.println(error.c_str());
      } else {
        // Utiliser des valeurs nulles si les clefs n'existent pas dans le JSON
        strlcpy(mqtt_server, doc["mqtt_server"] | "", sizeof(mqtt_server));
        strlcpy(mqtt_port_str, doc["mqtt_port_str"] | "", sizeof(mqtt_port_str));
        Serial.println("Configuration MQTT chargée depuis JSON.");
      }
      configFile.close();
    } else {
      Serial.println("Erreur ouverture fichier config (lecture).");
    }
  } else {
    Serial.println("Fichier de configuration MQTT non trouvé.");
  }
  Serial.print("Serveur MQTT chargé: "); Serial.println(mqtt_server);
  Serial.print("Port MQTT chargé: "); Serial.println(mqtt_port_str);
}

// Fonction pour sauvegarder la configuration MQTT sur LittleFS
void saveMqttConfig() {
  if (!LittleFS.begin(true)) {
    Serial.println("Erreur montage LittleFS pour sauvegarde config.");
    return;
  }
  Serial.print("Sauvegarde de la configuration MQTT..."); Serial.print(mqtt_server); Serial.print(mqtt_port_str);
  StaticJsonDocument<256> doc;
  doc["mqtt_server"] = mqtt_server;
  doc["mqtt_port_str"] = mqtt_port_str;

  File configFile = LittleFS.open(mqttConfigFilePath, "w");
  if (!configFile) {
    Serial.println("Échec de l'ouverture du fichier de config pour écriture");
    // LittleFS.end();
    return;
  }
  if (serializeJson(doc, configFile) == 0) {
    Serial.println(F("Échec de l'écriture dans le fichier de config"));
  } else {
    Serial.print("Configuration MQTT sauvegardée dans "); Serial.println((mqttConfigFilePath));
  }
  configFile.close();
}

void saveConfigCallback () {
  Serial.println("Callback WiFiManager: Connexion WiFi établie.");
  shouldSaveConfig = true;
}

void setupWifiManager() {
  WiFiManager wm;
  // wm.resetSettings();

  wm.setSaveConfigCallback(saveConfigCallback);
  wm.setConfigPortalTimeout(180);

  WiFiManagerParameter custom_mqtt_server("server", "Serveur MQTT", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "Port MQTT", mqtt_port_str, 6);

  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_port);

  if (force_config_portal) {
    Serial.println("Forçage du portail de configuration WiFiManager à cause d'échecs MQTT.");
    String apName = "ESP32-NReconfig-MQTT-" + String(WiFi.macAddress());  // Nom unique pour l'AP
    if (!wm.startConfigPortal(apName.c_str(), "password123")) {              // "password123" est le mot de passe de l'AP de config
      Serial.println("Échec de la connexion et timeout du portail de config atteint.");
      Serial.println("Redémarrage...");
      delay(3000);
      ESP.restart();  // Redémarre si le portail a expiré sans connexion
    } else {
      Serial.println("Connecté au WiFi!");
 
      strcpy(mqtt_server, custom_mqtt_server.getValue());
      strcpy(mqtt_port_str, custom_mqtt_port.getValue());
 
      Serial.println("Paramètres MQTT récupérés du portail:");
      Serial.print("Serveur: ");
      Serial.println(mqtt_server);
      Serial.print("Port: ");
      Serial.println(mqtt_port_str);
 
      if (shouldSaveConfig) {
        saveMqttConfig();
        shouldSaveConfig = false;
      }
    }
    force_config_portal = false;
  }else {
    String apName = "ESP32-Nano-Config-" + String(WiFi.macAddress());  // Nom unique pour l'AP
    if (!wm.autoConnect(apName.c_str(), "password123")) {              // "password123" est le mot de passe de l'AP de config
      Serial.println("Échec de la connexion et timeout du portail de config atteint.");
      Serial.println("Redémarrage...");
      delay(3000);
      ESP.restart();  // Redémarre si le portail a expiré sans connexion
    } else {
      Serial.println("Connecté au WiFi!");
 
      strcpy(mqtt_server, custom_mqtt_server.getValue());
      strcpy(mqtt_port_str, custom_mqtt_port.getValue());
 
      Serial.println("Paramètres MQTT récupérés du portail:");
      Serial.print("Serveur: ");
      Serial.println(mqtt_server);
      Serial.print("Port: ");
      Serial.println(mqtt_port_str);
 
      if (shouldSaveConfig) {
        saveMqttConfig();
        shouldSaveConfig = false;
      }
    }
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
  // Reception des ACK pour QoS1
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

    Serial.print("Message ID buzzer : ");
    Serial.println(id_buzzer);
    Serial.print("Valeur buzzer: ");
    Serial.println(value_buzzer);

  // Envoi de la confirmation à l'autre code pour QoS1
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
  // Boucle tant qu'on n'est pas connecté ET qu'on n'a pas dépassé le nombre d'essais
  while (!client.connected() && mqtt_connection_attempts < MAX_MQTT_ATTEMPTS) {
    Serial.print("Tentative de connexion MQTT (essai ");
    Serial.print(mqtt_connection_attempts + 1);
    Serial.print("/");
    Serial.print(MAX_MQTT_ATTEMPTS);
    Serial.print(")...");

    String clientIdString = clientID;
    clientIdString += String(random(0xffff), HEX);

    if (client.connect(clientIdString.c_str())) {
      Serial.println("connecté");
      client.subscribe(mqtt_confirm_card_topic);
      client.subscribe(mqtt_confirm_btn_topic);
      client.subscribe(mqtt_buzzer_topic);
      mqtt_connection_attempts = 0;
      return;
    } else {
      Serial.print("échec, rc=");
      Serial.print(client.state());
      Serial.println(" nouvelle tentative dans 5 secondes");
      mqtt_connection_attempts++;
      delay(5000);
    }
  }

  if (!client.connected() && mqtt_connection_attempts >= MAX_MQTT_ATTEMPTS) {
    Serial.println("Nombre maximal de tentatives de connexion MQTT atteint.");
    Serial.println("Déclenchement du portail de configuration au prochain redémarrage.");
    noinit_force_portal_flag = 1;

    Serial.println("Redémarrage pour activer le portail de configuration...");
    delay(1000);
    ESP.restart();
  }
}