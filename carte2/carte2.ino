#include "carte2.h"

int old_card = -1;
unsigned long scan_time = 0;
unsigned long lastTriggerTimeError = 0;

void setup() {
  Serial.begin(115200);
  
  // attend l'initialisation de serial
  unsigned long serialStartTime = millis();
  while (!Serial && (millis() - serialStartTime < 3000)) {
      delay(10);
  }

  Serial.print("Demarrage Carte 2 - Version "); Serial.println(currentFirmwareVersion_OTA);

  // --- Vérifier et gérer le flag .noinit ---
  if (noinit_first_run_check != NOINIT_FIRST_RUN_VALUE) {
    Serial.println("Premier demarrage a froid detecte (ou memoire .noinit corrompue). Initialisation des flags .noinit.");
    noinit_force_portal_flag = 0;
    noinit_first_run_check = NOINIT_FIRST_RUN_VALUE;
    force_config_portal = false;
  } else {
    Serial.print("Demarrage apres un reset logiciel. Valeur de noinit_force_portal_flag: ");
    Serial.println(noinit_force_portal_flag);
    if (noinit_force_portal_flag == 1) {
      Serial.println("Flag .noinit 'force_portal' detecte. Forcage du portail de configuration.");
      force_config_portal = true;
      noinit_force_portal_flag = 0;
    } else {
      Serial.println("Pas de flag .noinit de forcage detecte. Connexion normale.");
      force_config_portal = false;
    }
  }

  loadMqttConfig();

  setupWifiManager(); // Gère la connexion WiFi et peut mettre à jour mqtt_server/mqtt_port_str

  if (WiFi.isConnected()) {
    Serial.print("Adresse IP: ");
    Serial.println(WiFi.localIP());
    
    handleOTAUpdates(); // gère OTA update

    // Convertir mqtt_port_str en entier pour client.setServer
    int port_int = atoi(mqtt_port_str); 
    if (port_int == 0 && strcmp(mqtt_port_str, "0") != 0) {
      Serial.print("Conversion du port MQTT invalide: '"); Serial.print(mqtt_port_str); Serial.println("'. Utilisation de 1883 par défaut.");
      port_int = 1883; 
    } else if (port_int == 0) {
        Serial.println("Port MQTT est 0, utilisation de 1883 par défaut.");
        port_int = 1883;
    }
    
    Serial.print("Configuration du client MQTT avec Serveur: "); Serial.print(mqtt_server);
    Serial.print(", Port: "); Serial.println(port_int);
    
    client.setServer(mqtt_server, port_int);
    client.setCallback(callbackMqtt);
  } else {
    Serial.println("Pas connecté au WiFi après WiFiManager. Ne peut pas configurer MQTT.");
  } 

  SPI.begin();
  rfid.PCD_Init();

  setupButton(BTN_PIN, 5);
  
  pinMode(BUZZ_PIN, OUTPUT); 
  Serial.println("Setup terminé.");

  pinMode(A7, OUTPUT);
  digitalWrite(A7, HIGH); // pour ma batterie (peut être supprimé) 
}

void loop() {
  if (WiFi.isConnected()) {
    if (!client.connected()) {
      reconnect();
    }
    if (client.connected()){
        client.loop();
    }
  } else {
    Serial.println("WiFi déconnecté dans loop(). Tentative de reconnexion WiFi...");
    delay(5000);
  }

  now = millis();

  updateButtonState();

  if (value_buzzer == 1 && millis() - lastTriggerTimeError > 800) { // Pas content Erreur
    if (old_card == 0) old_card = -1;
    playErrorLoopSegment(BUZZ_PIN);
    lastTriggerTimeError = millis();
  }else if (value_buzzer == 2) { // Content bien arrivé
    if (old_card == 0) old_card = -1;
    playArrivalMelody(BUZZ_PIN);
    value_buzzer = 0;
  }else if (value_buzzer == 3) old_card = -1; // Fin de calibration (Musique à venir)

  // vérification changement état de bouton
  int currentButtonState;
  if (didButtonStateChange(currentButtonState)) {
    waitingForBtnAck=true;
    btnCount = 0;
    BtnMsgId++;
  }

  // vérification changement de carte
  int card_num = read_card();
  if (card_num != -1 && old_card!=card_num) 
  {
    Serial.print("New card :"); Serial.println(card_num);
    old_card = card_num;
    waitingForCardAck = true;
    cardCount = 0;
    scan_time = millis();
    CardMsgId++;
  }

  // Regarde si il y a des messages a envoyer tous les resendInterval ms
  if (now - lastSendTime > resendInterval){
    if (waitingForCardAck && cardCount<=10){ // Message carte
      StaticJsonDocument<100> doc;
      doc["id_card"] = CardMsgId;
      doc["value_card"] = old_card;
      doc["scan_time"] = scan_time;
      char msgBuffer[100];
      serializeJson(doc, msgBuffer);
      bool success = client.publish(mqtt_card_topic, msgBuffer);
      if (success) {
        Serial.println("Message carte publié avec succes");
      }else {
        Serial.println("Échec de publication carte");
      }
      lastSendTime = now;
      cardCount++;
    }
    if (waitingForBtnAck && btnCount <= 10){ // Message bouton
      StaticJsonDocument<100> doc;
      doc["id_button"] = BtnMsgId;
      doc["value_button"] = currentButtonState;
      char msgBuffer[100];
      serializeJson(doc, msgBuffer);
      bool success = client.publish(mqtt_button_topic, msgBuffer);
      if (success) {
        Serial.println("Message bouton publié avec succes");
      }else {
        Serial.println("Échec de publication bouton");
      }
      lastSendTime = now;
      btnCount++;
    }
  }
}