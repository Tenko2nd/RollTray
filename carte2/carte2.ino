#include "carte2.h"

int old_card = -1;
unsigned long lastTriggerTimeError = 0;


const int testLedPin = A7;
bool ledState = HIGH;

void setup() {
  Serial.begin(115200);
  unsigned long startTime = millis();
  const unsigned long timeout = 5000;

  while (!Serial && (millis() - startTime < timeout)) {
    delay(10);
  }


  Serial.print("Demarrage Carte 2 - Version "); Serial.println(currentFirmwareVersion_OTA);


  pinMode(testLedPin, OUTPUT);
  digitalWrite(testLedPin, ledState);

  unsigned long serialStartTime = millis();
  while (!Serial && (millis() - serialStartTime < 3000)) {
      delay(10);
  }

  loadMqttConfig(); // CHARGER LA CONFIG ICI

  setupWifiManager(); // Gère la connexion WiFi et peut mettre à jour mqtt_server/mqtt_port_str

  if (WiFi.isConnected()) {
    Serial.print("Adresse IP: ");
    Serial.println(WiFi.localIP());
    
    handleOTAUpdates();

    // Convertir mqtt_port_str en entier pour client.setServer
    int port_int = atoi(mqtt_port_str); 
    if (port_int == 0 && strcmp(mqtt_port_str, "0") != 0) { // strcmp pour le cas où "0" est un port valide (peu probable)
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

  SPI.begin(); // Initialise le bus SPI
  rfid.PCD_Init(); // Initialise le lecteur MFRC522

  setupButton(BTN_PIN, 5);
  
  pinMode(BUZZ_PIN, OUTPUT); 
  Serial.println("Setup terminé.");
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
    playArrivalMelody(BUZZ_PIN);
    value_buzzer = 0;
  }else if (value_buzzer == 3) old_card = -1; // Musique à venir

  int currentButtonState; // This will be filled by the function
  if (didButtonStateChange(currentButtonState)) {
    waitingForBtnAck=true;
    btnCount = 0;
    BtnMsgId++;
  }

  int card_num = read_card();
  if (card_num != -1 && old_card!=card_num) 
  {
    Serial.print("New card :"); Serial.println(card_num);
    old_card = card_num;
    waitingForCardAck = true;
    cardCount = 0;
    CardMsgId++;
  }

  if (now - lastSendTime > resendInterval){
    if (waitingForCardAck && cardCount<=10){
      StaticJsonDocument<100> doc;
      doc["id_card"] = CardMsgId;
      doc["value_card"] = old_card;
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
    if (waitingForBtnAck && btnCount <= 10){
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