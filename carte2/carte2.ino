#include "carte2.h"

int old_card = -1;

void setup() {
  Serial.begin(9600);
  while (!Serial); 
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  SPI.begin(); // Initialise le bus SPI
  rfid.PCD_Init(); // Initialise le lecteur MFRC522

  setupButton(BTN_PIN, 5);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  now = millis();

  updateButtonState();

  int currentButtonState; // This will be filled by the function
  if (didButtonStateChange(currentButtonState)) {
    Serial.print("Button state CHANGED! New state: ");
    if (currentButtonState == LOW) { // Assuming INPUT_PULLUP, LOW means pressed
      Serial.println("PRESSED (LOW)");
      // --- Your code for when the button becomes PRESSED ---
    } else {
      Serial.println("RELEASED (HIGH)");
      // --- Your code for when the button becomes RELEASED ---
    }
  }

  int card_num = read_card();
  if (card_num != -1 && old_card!=card_num) 
  {
    Serial.print("New card :"); Serial.println(card_num);
    old_card = card_num;
    waitingForAck = true;
    messageId++;
  }
  if (waitingForAck && (now - lastSendTime > resendInterval)){
    StaticJsonDocument<100> doc;
    doc["id"] = messageId;
    doc["value"] = old_card;
    char msgBuffer[100];
    serializeJson(doc, msgBuffer);
    bool success = client.publish(mqtt_publish_topic, msgBuffer);
    if (success) {
      Serial.println("Message publié avec succes");
    }else {
      Serial.println("Échec de publication");
    }
    lastSendTime = now;
  }

  delay(20);
}