#include "iot.h" // This MUST be included to link declarations with definitions

// --- DEFINITIONS of ALL Global Variables and Objects ---

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

bool shouldSaveConfig = false;
const char *mqttConfigFilePath = "/mqtt_config.json";

int mqtt_connection_attempts = 0;
const int MAX_MQTT_ATTEMPTS = 5; // Nombre maximal d'essais avant de forcer le portail
bool force_config_portal = false;

// --- Function DEFINITIONS ---
// Fonction pour charger la configuration MQTT depuis LittleFS
void loadMqttConfig() {
  if (!LittleFS.begin(true)) { // true pour formater si échec du montage
    Serial.println("Erreur montage LittleFS pour chargement config.");
    return;
  }
  Serial.println("LittleFS monté.");

  if (LittleFS.exists(mqttConfigFilePath)) {
    Serial.print("Lecture du fichier de configuration MQTT: ");
    Serial.println(mqttConfigFilePath);
    File configFile = LittleFS.open(mqttConfigFilePath, "r");
    if (configFile) {
      StaticJsonDocument<256> doc; // Ajustez la taille si vous ajoutez plus de paramètres
      DeserializationError error = deserializeJson(doc, configFile);
      if (error) {
        Serial.print(F("deserializeJson() a échoué: "));
        Serial.println(error.c_str());
      } else {
        // Utiliser des valeurs par défaut si les clés n'existent pas dans le JSON
        strlcpy(mqtt_server, doc["mqtt_server"] | "192.168.1.100", sizeof(mqtt_server));
        strlcpy(mqtt_port_str, doc["mqtt_port_str"] | "1883", sizeof(mqtt_port_str)); // Notez le _str
        Serial.println("Configuration MQTT chargée depuis JSON.");
      }
      configFile.close();
    } else {
      Serial.println("Erreur ouverture fichier config (lecture).");
    }
  } else {
    Serial.println("Fichier de configuration MQTT non trouvé. Utilisation des valeurs par défaut et sauvegarde.");
    // Si le fichier n'existe pas, sauvegardons les valeurs par défaut actuelles
    saveMqttConfig(); 
  }
  // LittleFS.end(); // Pas besoin de fermer ici si on va peut-être le réutiliser rapidement
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
  doc["mqtt_port_str"] = mqtt_port_str; // Notez le _str

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
  // LittleFS.end(); // Peut être fermé si on n'en a plus besoin immédiatement
}

void saveConfigCallback () {
  Serial.println("Callback WiFiManager: Connexion WiFi établie.");
  shouldSaveConfig = true; // Mettre le drapeau pour sauvegarder
}

void setupWifiManager() {
  WiFiManager wm;
  // wm.resetSettings(); // Pour tests seulement

  if (force_config_portal) {
    Serial.println("Forçage du portail de configuration WiFiManager à cause d'échecs MQTT.");
    // Vous pouvez choisir un nom d'AP différent pour indiquer que c'est une re-configuration
    wm.setConfigPortalTimeout(300); // Timeout plus long pour la reconfiguration
    if (!wm.startConfigPortal("ESP32-ReConfig-MQTT", "password123")) {
        Serial.println("Portail de configuration forcé a expiré ou a échoué. Redémarrage.");
        delay(3000);
        ESP.restart();
    }
    // Si l'utilisateur configure et sauvegarde, autoConnect sera appelé implicitement
    // ou la connexion sera établie, et les nouveaux paramètres seront disponibles.
    // On réinitialise le drapeau après une tentative de portail
    force_config_portal = false; 
  }

  wm.setSaveConfigCallback(saveConfigCallback);
  wm.setConfigPortalTimeout(180);

  // Important: Passer les variables CHAR ARRAY à WiFiManagerParameter
  // elles seront remplies avec les valeurs du portail si l'utilisateur les modifie.
  WiFiManagerParameter custom_mqtt_server("server", "Serveur MQTT", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "Port MQTT", mqtt_port_str, 6); // Utiliser mqtt_port_str

  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_port);

  String apName = "ESP32-Nano-Config-" + String(WiFi.macAddress());
  if (!wm.autoConnect(apName.c_str(), "password123")) {
    Serial.println("Échec de la connexion WiFiManager et timeout.");
    delay(3000);
    ESP.restart();
  } else {
    Serial.println("Connecté au WiFi via WiFiManager!");

    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_port_str, custom_mqtt_port.getValue());

    Serial.println("Paramètres MQTT récupérés du portail:");
    Serial.print("Serveur: "); Serial.println(mqtt_server);
    Serial.print("Port: "); Serial.println(mqtt_port_str);

    // Si le callback a été appelé (nouvelle config WiFi ou paramètres MQTT modifiés)
    if (shouldSaveConfig) {
      saveMqttConfig();
      shouldSaveConfig = false;
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
      mqtt_connection_attempts = 0; // Réinitialiser les tentatives en cas de succès
      return; // Sortir de la fonction si connecté
    } else {
      Serial.print("échec, rc=");
      Serial.print(client.state());
      Serial.println(" nouvelle tentative dans 5 secondes");
      mqtt_connection_attempts++;
      delay(5000);
    }
  }

  // Si on sort de la boucle while, c'est soit qu'on est connecté (improbable à cause du return),
  // soit qu'on a dépassé le nombre MAX_MQTT_ATTEMPTS
  if (!client.connected() && mqtt_connection_attempts >= MAX_MQTT_ATTEMPTS) {
    Serial.println("Nombre maximal de tentatives de connexion MQTT atteint.");
    Serial.println("Déclenchement du portail de configuration au prochain redémarrage.");
    force_config_portal = true; // Mettre le drapeau

    Serial.println("Redémarrage pour activer le portail de configuration...");
    delay(1000);
    ESP.restart(); // Redémarrer pour que setup() puisse vérifier force_config_portal
  }
}