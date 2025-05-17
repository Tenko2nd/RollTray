#include "carte1.h"


void setup() {
  Serial.begin(9600);

  // ---- Ecran OLED + initialisation ----
  display.begin();
  afficher_setup();

  // ---- PINs du convoyeur + intialisation ---
  pinMode(PIN_CTRL_A1, OUTPUT);
  pinMode(PIN_SELECT_1, OUTPUT);
  pinMode(PIN_SELECT_2, OUTPUT);
  pinMode(PIN_MASTER_EN, OUTPUT);
  stopConveyor();
  delay(500);

  // ---- Pins des LEDs + initialisation ----
  pinMode(ledVert, OUTPUT);
  pinMode(ledRouge, OUTPUT);
  digitalWrite(ledVert, LOW);
  delay(500);
  digitalWrite(ledRouge, LOW);
  delay(500);

  // --- Initialisation de la partie IoT (WiFi + protocole MQTT) ---
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}


void loop() {

  now = millis();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // [PARTIE KEYPAD : ACTIONS LORSQUE LE KEYPAD EST PRESSÉ]


  // ---- Récupération de la valeur du keypad -----
  char current_key = kypd.getKey();

  // ---- Si une touche est pressée, alors... -----
  if (current_key != 0) {

    // ---- Si le bouton "E" (bouton_envoi) est appuyé et que le BP est appuyé-----
    if (current_key == bouton_envoi && value_button == etat_bp_appuye) {

      // ----- Si on est en attente de confirmation, si la dernière key n'était pas "interdite" (donc 1, 2 ou 3) et s'il n'y a aucun
      // retour en cours, alors...
      if (attente_confirmation && !is_forbidden(last_key) && conveyorToBase && !retour_en_cours) {
        afficher_envoi(last_key);
        attente_confirmation = false;  // On n'attend plus de confirmation
        envoi_effectue = true;         // L'envoi a bien été effectué

        digitalWrite(ledVert, HIGH);
        digitalWrite(ledRouge, LOW);
        moveForward(1);
        delay(500);
        conveyorToBase = false;  // Le convoyeur a quitté la base, il a bien été envoyé
      }
    }

    // --- Si un bouton qui n'est pas "interdit" est appuyé (autre que 1, 2 ou 3) ---
    else if (!is_forbidden(current_key) && current_key != bouton_envoi && conveyorToBase && !retour_en_cours) {
      last_key = current_key;
      afficher_en_cours(last_key);
      attente_confirmation = true;  // On attend une confirmation en appuyant sur le bouton "E"
      envoi_effectue = false;
    }

    // --- Si un bouton interdit est appuyé (autre que 1, 2 ou 3) ---
    else {
      afficher_interdit();
      attente_confirmation = false;
      envoi_effectue = false;
    }
  }

  int last_key_int = last_key - '0';  // Conversion de la "last_key" en int



  // [PARTIE PLATEAU SUR CONVOYEUR]

  // --- Si la carte correspond bien au n° du keyboard et que le BP est appuyé, musique joyeuse + affichage ---
  // ✩ Bonne arrivée du plateau à sa carte ✩
  if ((last_key_int == value_card && value_button == etat_bp_appuye)) {
    afficher_plateau_arrive();
    current_value_buzzer = JOYFUL_MUSIC;

    // Envoi du message à la carte n°2 pour activer le buzzer
    if ((current_value_buzzer != old_value_buzzer)) {
      waitingForBzrAck = true;
      BzrMsgId++;
      send_message_buzzer_mqtt(current_value_buzzer);
      old_value_buzzer = current_value_buzzer;
    }

    // On stoppe le convoyeur + on allume les 2 LEDs
    brakeConveyor();
    delay(500);
    stopConveyor();
    delay(500);
    digitalWrite(ledVert, HIGH);
    digitalWrite(ledRouge, HIGH);
  }

  // --- Si la valeur de la carte correspond à la valeur du keypad et que le BP est relâché (ex : plat pris)
  // ✩ Signal pour le retour à la base ✩
  else if (last_key_int == value_card && value_button == etat_bp_relache) {
    afficher_retour();
    current_value_buzzer = STOP_MUSIC;

    if ((current_value_buzzer != old_value_buzzer)) {
      waitingForBzrAck = true;
      BzrMsgId++;
      send_message_buzzer_mqtt(current_value_buzzer);
      old_value_buzzer = current_value_buzzer;
    }

    digitalWrite(ledVert, LOW);
    digitalWrite(ledRouge, HIGH);
    moveBackward(128);
    afficher_retour();
    delay(500);
    conveyorToBase = true;
    retour_en_cours = true;
  }


  // --- Si la valeur de la carte scannée est 0 (base) et que le convoyeur est retourné à la base ---
  // ✩ Le plateau est bien retourné à la base, réinitialisation ✩
  else if (value_card == 0 && conveyorToBase) {
    afficher_plateau_arrive();
    brakeConveyor();
    delay(500);
    stopConveyor();
    delay(500);
    digitalWrite(ledVert, LOW);
    digitalWrite(ledRouge, LOW);
    reinitialize();  // Réinitialisation de toutes les variables
    afficher_setup();
    retour_en_cours = false;  // Aucun retour n'est en cours
  }

  /// --- ERREUR : si la carte ne correspond pas au n° du keypad et que le convoyeur n'est pas à la base ---
  // ✩ Cas où le scnénario idéal est perturbé ✩
  else if (last_key_int != value_card && !conveyorToBase) {

    // 1) S'il n'y a rien sur le BP (ex : si la personne a pris le plat du plateau alors que ce n'est pas le sien)
    if (value_button == etat_bp_relache) {
      wrong_card = true;
      afficher_erreur();
      brakeConveyor();
      delay(500);

      // Envoi du signal pour le buzzer à la carte n°2
      current_value_buzzer = NOT_HAPPY_MUSIC;
      if ((current_value_buzzer != old_value_buzzer)) {
        waitingForBzrAck = true;
        BzrMsgId++;
        send_message_buzzer_mqtt(current_value_buzzer);
        old_value_buzzer = current_value_buzzer;
      }

    }

    // 2) Si le BP est de nouveau pressé mais que l'on est toujours sur la mauvaise carte (ex : si le plat a été reposé)
    else if (value_button == etat_bp_appuye && wrong_card) {
      afficher_retour_erreur();
      current_value_buzzer = STOP_MUSIC;

      // Envoi du signal pour le buzzer à la carte n°2 (arrêt musique)
      if ((current_value_buzzer != old_value_buzzer)) {
        waitingForBzrAck = true;
        BzrMsgId++;
        send_message_buzzer_mqtt(current_value_buzzer);
        old_value_buzzer = current_value_buzzer;
      }

      // Le plateau revient à la base (en guise de sécurité)
      digitalWrite(ledVert, LOW);
      digitalWrite(ledRouge, HIGH);
      moveBackward(128);
      afficher_retour();
      delay(500);
      wrong_card = false;
      conveyorToBase = true;
      retour_en_cours = true;
    }
  }


  if (now - lastSendTime > resendInterval) {
    if (waitingForBzrAck && buzzerCount <= 10) {
      send_message_buzzer_mqtt(current_value_buzzer);
    }
  }
}
