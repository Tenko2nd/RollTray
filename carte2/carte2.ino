#include "carte2.h"

int old_card = -1;
unsigned long lastTriggerTimeError = 0;

void setup() {
  Serial.begin(115200);
  unsigned long serialStartTime = millis();
  while (!Serial && (millis() - serialStartTime < 3000)) {
      delay(10);
  }
  
  setupWifiManager();

  if (WiFi.isConnected()) {
    Serial.print("Adresse IP: ");
    Serial.println(WiFi.localIP());
    int port = atoi(mqtt_port); // Convertir la chaîne du port en entier
    if (port == 0) port = 1883; // Valeur par défaut si la conversion échoue
    client.setServer(mqtt_server, port);
    client.setCallback(callbackMqtt);
  } else {
    Serial.println("Pas connecté au WiFi après WiFiManager. Ne peut pas configurer MQTT.");
    // L'ESP devrait avoir redémarré ou être bloqué si autoConnect a échoué après timeout.
  }

  SPI.begin(); // Initialise le bus SPI
  rfid.PCD_Init(); // Initialise le lecteur MFRC522

  setupButton(BTN_PIN, 5);
  
  pinMode(BUZZ_PIN, OUTPUT); 
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  now = millis();

  updateButtonState();

  if (value_buzzer == 1 && millis() - lastTriggerTimeError > 800) { // Pas content Erreur
    if (old_card == 0) old_card = -1;
    playErrorLoopSegment(BUZZ_PIN);
    lastTriggerTimeError = millis();
  }else if (value_buzzer == 2) { // Content bien arrivé
    playArrivalMelody(BUZZ_PIN);
    value_buzzer = 0;
  }

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