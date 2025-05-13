#include "carte1.h"


void setup() {
  Serial.begin(9600);
  setup_wifi();
  display.begin();
  afficher_setup();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  pinMode(PIN_CTRL_A1, OUTPUT);
  pinMode(PIN_SELECT_1, OUTPUT);
  pinMode(PIN_SELECT_2, OUTPUT);
  pinMode(PIN_MASTER_EN, OUTPUT);
  stopConveyor();
  delay(1000);
}


void loop() {

  // value_button = 0 : relâché
  // value_button = 1 : appuyé

  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  // Serial.println(value_button);

  char current_key = kypd.getKey();


  if (current_key != 0) {

    // Si le bouton "E" est appuyé
    if (current_key == bouton_envoi && value_button == 1) {


      if (attente_confirmation && !is_forbidden(last_key)) {
        afficher_envoi(last_key);
        attente_confirmation = false;
        envoi_effectue = true;

        moveForward(1);  // Lent
        delay(500);
      }
    }

    // Si un bouton qui n'est pas interdit est appuyé (1, 2 ou 3)
    else if (!is_forbidden(current_key)) {
      last_key = current_key;
      afficher_en_cours(last_key);
      attente_confirmation = true;
      envoi_effectue = false;
    }

    // Si un bouton interdit est appuyé
    else {
      afficher_interdit();
      attente_confirmation = false;
      envoi_effectue = false;
    }
  }

  int last_key_int = last_key - '0';


  if ((last_key_int == value_card && value_button == 1) || value_card == 0) {
    Serial.println("Carte détectée");
    brakeConveyor();
    delay(500);
    stopConveyor();
    delay(500);

    if (value_card == 0){
      value_card = -1;
    }
  }

  if (last_key_int == value_card && value_button == 0) {
    moveBackward(128);  // Lent
    delay(500);
  }
}
