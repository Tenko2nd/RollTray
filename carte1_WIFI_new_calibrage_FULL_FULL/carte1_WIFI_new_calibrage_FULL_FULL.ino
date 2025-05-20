#include "carte1.h"


void setup() {
  Serial.begin(9600);

  // ---- Ecran OLED + initialisation ----
  display.begin();

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
  delay(100);
  digitalWrite(ledRouge, LOW);
  delay(100);

  // --- Initialisation de la partie IoT (WiFi + protocole MQTT) ---


  setupWifiManager();
  affichage_setup_wifi();

  if (WiFi.isConnected()) {
    Serial.print("Adresse IP: ");
    Serial.println(WiFi.localIP());
    int port = atoi(mqtt_port);  // Convertir la chaîne du port en entier
    if (port == 0) port = 1883;  // Valeur par défaut si la conversion échoue
    client.setServer(mqtt_server, port);
    client.setCallback(callback);
  } else {
    affichage_erreur_wifi();
    Serial.println("Pas connecté au WiFi après WiFiManager. Ne peut pas configurer MQTT.");
    // L'ESP devrait avoir redémarré ou être bloqué si autoConnect a échoué après timeout.
  }

  affichage_calibrage_debut();

  // Définition des intervalles de temps
}


void loop() {

  now = millis();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  if (!calibrageFait) {
    setup_conveyor();
  }

  else if (calibrageFait) {


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
          current_affichage = ENVOI;
          if (current_affichage != old_affichage) {
            afficher_envoi(last_key);
            old_affichage = current_affichage;
          }
          attente_confirmation = false;  // On n'attend plus de confirmation

          digitalWrite(ledVert, HIGH);
          digitalWrite(ledRouge, LOW);
          moveForward(1);
          delay(100);

          tempsDepart = millis();
          conveyorToBase = false;  // Le convoyeur a quitté la base, il a bien été envoyé
          envoi_effectue = true;   // L'envoi a bien été effectué
        }
      }

      // --- Si un bouton qui n'est pas "interdit" est appuyé (autre que 1, 2 ou 3) ---
      else if (!is_forbidden(current_key) && current_key != bouton_envoi && conveyorToBase && !retour_en_cours) {
        last_key = current_key;

        current_affichage = EN_COURS;
        if (current_affichage != old_affichage) {
          afficher_en_cours(last_key);
          old_affichage = current_affichage;
        }

        attente_confirmation = true;  // On attend une confirmation en appuyant sur le bouton "E"
        envoi_effectue = false;
      }

      // --- Si un bouton interdit est appuyé (autre que 1, 2 ou 3) ---
      else {
        current_affichage = INTERDIT;
        if (current_affichage != old_affichage) {
          afficher_interdit();
          old_affichage = current_affichage;
        }
        attente_confirmation = false;
        envoi_effectue = false;
      }
    }

    // [PARTIE CARTES ET CONVOYEUR]

    int last_key_int = last_key - '0';

    // Affichage du numéro de la carte passée tout au long de la livraison
    if ((last_key_int != value_card && value_button == etat_bp_appuye && !retour_en_cours && envoi_effectue)) {

      choose_current_affichage_card(value_card);

      if (current_affichage != old_affichage) {
        afficher_carte_convoyeur(value_card);
        old_affichage = current_affichage;
      }
    }

    // --- Si la carte correspond bien au n° du keyboard et que le BP est appuyé ET que le retour n'est pas en cours ---
    // ✩ Phase d'arrivée du plateau ou d'attente de prise/timeout ✩
    // if (last_key_int == value_card && value_button == etat_bp_appuye && !retour_en_cours) {
    if (triggerLivraison(last_key_int)) {
      if (!livraison_faite) {
        livraison_faite = true;
        envoi_effectue = false;

        current_affichage = PLATEAU_ARRIVE;
        if (current_affichage != old_affichage) {
          afficher_plateau_arrive();
          old_affichage = current_affichage;
        }

        current_value_buzzer = JOYFUL_MUSIC;
        if (current_value_buzzer != old_value_buzzer) {
          waitingForBzrAck = true;
          BzrMsgId++;
          send_message_buzzer_mqtt(current_value_buzzer);
          old_value_buzzer = current_value_buzzer;
        }

        brakeConveyor();
        delay(100);
        stopConveyor();
        delay(100);
        digitalWrite(ledVert, HIGH);
        digitalWrite(ledRouge, HIGH);
      }
    }

    // --- Étape 2 : Gestion du timer d’attente / retour plateau ---
    if (livraison_faite && !retour_en_cours) {
      if (!timerAttentePriseDemarre) {
        attente_retour = millis();
        timerAttentePriseDemarre = true;
        Serial.println("Timer d'attente démarré");
      }

      if (timerAttentePriseDemarre && (millis() - attente_retour >= ATTENTE)) {
        Serial.println("Timeout ! Le plat n'a pas été pris");

        current_affichage = RETOUR;
        if (current_affichage != old_affichage) {
          afficher_retour_timeout();
          old_affichage = current_affichage;
        }

        current_value_buzzer = STOP_MUSIC;
        if (current_value_buzzer != old_value_buzzer) {
          waitingForBzrAck = true;
          BzrMsgId++;
          send_message_buzzer_mqtt(current_value_buzzer);
          old_value_buzzer = current_value_buzzer;
        }

        digitalWrite(ledVert, LOW);
        digitalWrite(ledRouge, HIGH);
        moveBackward(128);
        delay(100);

        retour_en_cours = true;
        timerAttentePriseDemarre = false;
        livraison_faite = false;
      }
    }


    // --- Si le plat est bien pris (BP relâché) et avant qu'un retour ne soit initié ---
    if (livraison_faite && value_button == etat_bp_relache && !retour_en_cours) {
      Serial.println("Plat pris !");
      current_affichage = RETOUR;
      if (current_affichage != old_affichage) {
        afficher_retour();
        old_affichage = current_affichage;
      }
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
      delay(100);
      retour_en_cours = true;
      retour_ideal = true;
      timerAttentePriseDemarre = false;
      livraison_faite = false;
    }


    // --- Si la valeur de la carte scannée est 0 (base) et que le convoyeur est retourné à la base ---
    // ✩ Le plateau est bien retourné à la base, réinitialisation ✩
    if (value_card == 0 && retour_en_cours) {
      current_affichage = PLATEAU_ARRIVE;
      if (current_affichage != old_affichage) {
        afficher_plateau_arrive();
        old_affichage = current_affichage;
      }
      brakeConveyor();
      delay(100);
      stopConveyor();
      delay(100);
      digitalWrite(ledVert, LOW);
      digitalWrite(ledRouge, LOW);
      reinitialize_variables();  // Réinitialise toutes les variables
    }


    /// --- ERREUR : si la carte ne correspond pas au n° du keypad et que le convoyeur n'est pas à la base ---
    // ✩ Cas où le scnénario idéal est perturbé ✩
    else if (last_key_int != value_card && !conveyorToBase && !livraison_faite && !retour_ideal) {

      // 1) S'il n'y a rien sur le BP (ex : si la personne a pris le plat du plateau alors que ce n'est pas le sien)
      if (value_button == etat_bp_relache) {
        wrong_card = true;
        current_affichage = ERREUR;
        if (current_affichage != old_affichage) {
          afficher_erreur();
          old_affichage = current_affichage;
        }
        digitalWrite(ledVert, HIGH);
        digitalWrite(ledRouge, HIGH);
        brakeConveyor();
        delay(100);

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
        current_affichage = RETOUR_ERREUR;
        if (current_affichage != old_affichage) {
          afficher_retour_erreur();
          old_affichage = current_affichage;
        }
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
        delay(500);
        wrong_card = false;
        //conveyorToBase = true;
        retour_en_cours = true;
      }
    }

    // [S'IL N'Y A AUCUNE CARTE QUI N'A ETE DETECTEE SUR LE CHEMIN]
    if (value_card == -2 && !wrong_card) {
      current_affichage = RETOUR;
      if (current_affichage != old_affichage) {
        afficher_retour_nodetection();
        moveBackward(128);
        old_affichage = current_affichage;
      }

      retour_en_cours = true;
      retour_ideal = true;
      value_card == -1;
    }


    if (now - lastSendTime > resendInterval) {
      if (waitingForBzrAck && buzzerCount <= 10) {
        send_message_buzzer_mqtt(current_value_buzzer);
      }
    }
  }
}


bool triggerLivraison(int last_key_int) {
  return ((checkTimerCard1(last_key_int) || checkTimerCard2(last_key_int) || checkTimerCard3(last_key_int) || checkTimerCard4(last_key_int) || last_key_int == value_card)
  && value_button == etat_bp_appuye && !retour_en_cours && envoi_effectue && !wrong_card);
}