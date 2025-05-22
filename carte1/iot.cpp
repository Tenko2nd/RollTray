#include "iot.h"

const int CALIBRAGE_FAIL = 3;
const int JOYFUL_MUSIC = 2;
const int NOT_HAPPY_MUSIC = 1;
const int STOP_MUSIC = 0;

char mqtt_server[40] = "";
char mqtt_port_str[6] = "";

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

unsigned long scan_time = 0;


WiFiClient espClient;
PubSubClient client(espClient);

bool shouldSaveConfig = false;

// Variables pour LittleFS

const char* mqttConfigFilePath = "/mqtt_config.json";  // fichier config json pour littlefs

int mqtt_connection_attempts = 0;
const int MAX_MQTT_ATTEMPTS = 5;  // Nombre maximal d'essais avant de forcer le portail
bool force_config_portal = false;
// Définition des variables .noinit
uint8_t noinit_force_portal_flag;
uint32_t noinit_first_run_check;
const uint32_t NOINIT_FIRST_RUN_VALUE = 0xCAFEBABE;

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
  Serial.print("Serveur MQTT chargé: ");
  Serial.println(mqtt_server);
  Serial.print("Port MQTT chargé: ");
  Serial.println(mqtt_port_str);
}

// Fonction pour sauvegarder la configuration MQTT sur LittleFS
void saveMqttConfig() {
  if (!LittleFS.begin(true)) {
    Serial.println("Erreur montage LittleFS pour sauvegarde config.");
    return;
  }
  Serial.print("Sauvegarde de la configuration MQTT...");
  Serial.print(mqtt_server);
  Serial.print(mqtt_port_str);
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
    Serial.print("Configuration MQTT sauvegardée dans ");
    Serial.println((mqttConfigFilePath));
  }
  configFile.close();
}


void saveConfigCallback() {
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
  wm.setConfigPortalTimeout(180);  // 3 minutes

  // Ajouter des paramètres personnalisés pour MQTT
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

  // Tente de se connecter au WiFi. S'il échoue, démarre le portail de configuration.
  // Le nom du point d'accès sera "AutoConnectAP" par défaut, ou celui que vous spécifiez.
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
    scan_time = doc["scan_time"];

    // Serial.print("Message ID card : ");
    // Serial.println(id_card);
    // Serial.print("Valeur card : ");
    // Serial.println(value_card);

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



// (Re)connexion aux topics MQTT
void reconnect() {
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

      // S'abonner au topic des cartes
      if (client.subscribe(mqtt_card_topic)) {
        Serial.print("Abonné à : ");
        Serial.println(mqtt_card_topic);
      } else {
        affichage_erreur_mqtt();
        Serial.print("Échec abonnement à ");
        Serial.println(mqtt_card_topic);
      }

      // S'abonner au topic des boutons
      if (client.subscribe(mqtt_button_topic)) {
        Serial.print("Abonné à : ");
        Serial.println(mqtt_button_topic);
      } else {
        affichage_erreur_mqtt();
        Serial.print("Échec abonnement à ");
        Serial.println(mqtt_button_topic);
      }

      if (client.subscribe(mqtt_confirm_buzzer_topic)) {
        Serial.print("Abonné à : ");
        Serial.println(mqtt_confirm_buzzer_topic);
      } else {
        affichage_erreur_mqtt();

        Serial.print("Échec abonnement à ");
        Serial.println(mqtt_confirm_buzzer_topic);
      }

    } else {
      affichage_setup_mqtt();  // Ecran d'affichage sur OLED
      Serial.print("Échec, rc=");
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