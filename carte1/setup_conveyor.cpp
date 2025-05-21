#include "setup_conveyor.h"

bool carte1_verifiee = false;
bool carte2_verifiee = false;
bool carte3_verifiee = false;
bool carte4_verifiee = false;

bool calibrageFait = false;
bool calibrageEnCours = false;

bool attenteNouvelleCarte0 = false;

bool bascule_etat_0 = false;

const unsigned long TEMPS_MAX_CALIBRAGE = 10000;
unsigned long temps_debut_calibrage = 0;

unsigned long t0 = 0;
unsigned long t1 = 0;
unsigned long t2 = 0;
unsigned long t3 = 0;
unsigned long t4 = 0;

unsigned long t01 = 0;
unsigned long t12 = 0;
unsigned long t23 = 0;
unsigned long t34 = 0;

void setup_conveyor() {

  if (!calibrageEnCours && value_card == 0 && !attenteNouvelleCarte0) {

    Serial.println("LANCEMENT DU CALIBRAGE...");
    moveForward(1);
    delay(100);
    temps_debut_calibrage = millis();

    digitalWrite(ledVert, HIGH);
    digitalWrite(ledRouge, HIGH);

    bascule_etat_0 = false;

    calibrageEnCours = true;

    attenteNouvelleCarte0 = true;

    t0 = scan_time;

    Serial.print("Temps t0 enregistré : ");
    Serial.println(t0);

  }

  else if (calibrageEnCours) {

    if (!carte1_verifiee && value_card == 1) {
      carte1_verifiee = true;
      t1 = scan_time;
      Serial.print("Temps t1 enregistré : ");
      Serial.println(t1);
      affichage_calibrage(value_card);
    }

    else if (!carte2_verifiee && value_card == 2) {
      carte2_verifiee = true;
      t2 = scan_time;
      Serial.print("Temps t2 enregistré : ");
      Serial.println(t2);
      affichage_calibrage(value_card);
    }

    else if (!carte3_verifiee && value_card == 3) {
      carte3_verifiee = true;
      t3 = scan_time;
      Serial.print("Temps t3 enregistré : ");
      Serial.println(t3);
      affichage_calibrage(value_card);
    }

    else if (!carte4_verifiee && value_card == -2) {
      carte4_verifiee = true;
      t4 = scan_time;
      Serial.print("Temps t4 enregistré : ");
      Serial.println(t4);
      affichage_calibrage(value_card);
    }



    if (toutesCartesVerifiees()) {
      moveBackward(128);
      delay(500);

      if (value_card == 0) {
        brakeConveyor();
        delay(100);
        stopConveyor();
        delay(100);

        affichage_calibrage_terminee();
        delay(2000);
        reinitialize_variables();

        calibrageFait = true;
        calibrageEnCours = false;

        attenteNouvelleCarte0 = false;

        if (calibrageFait) {

          t01 = t1 - t0;
          t12 = t2 - t1;
          t23 = t3 - t2;
          t34 = t4 - t3;

          current_value_buzzer = STOP_MUSIC;
          if ((current_value_buzzer != old_value_buzzer)) {
            waitingForBzrAck = true;
            BzrMsgId++;
            send_message_buzzer_mqtt(current_value_buzzer);
            old_value_buzzer = current_value_buzzer;
          }  // envoi d'un signal à carte n°2 (= 0)
        }
      }
    }


    if ((millis() - temps_debut_calibrage > TEMPS_MAX_CALIBRAGE) && !toutesCartesVerifiees()) {
      delay(1000);
      Serial.println("TEMPS MAX CALIBRAGE !! TIMEOUT");

      current_value_buzzer = CALIBRAGE_FAIL;
      if ((current_value_buzzer != old_value_buzzer)) {
        waitingForBzrAck = true;
        BzrMsgId++;
        send_message_buzzer_mqtt(current_value_buzzer);
        old_value_buzzer = current_value_buzzer;
      }  // envoi d'un signal à carte n°2 disant que le calibrage a échoué

      moveBackward(128);

      if (!bascule_etat_0) {
        value_card = -1;
        bascule_etat_0 = true;
      }

      if (value_card == 0 && bascule_etat_0) {
        brakeConveyor();
        delay(100);
        stopConveyor();
        delay(100);

        attenteNouvelleCarte0 = true;

        calibrageFait = false;
        calibrageEnCours = false;
        value_card = -1;

        current_value_buzzer = STOP_MUSIC;
        if ((current_value_buzzer != old_value_buzzer)) {
          waitingForBzrAck = true;
          BzrMsgId++;
          send_message_buzzer_mqtt(current_value_buzzer);
          old_value_buzzer = current_value_buzzer;
        }  // envoi d'un signal à carte n°2 (= 0)
      }
    }
  }


  else if (attenteNouvelleCarte0 && value_card != 0) {
    // Dès que la carte change, on attend à nouveau un nouveau 0 propre
    attenteNouvelleCarte0 = false;
  }
}



void reinitialize_variables() {

  digitalWrite(ledVert, LOW);
  digitalWrite(ledRouge, LOW);

  afficher_setup();

  value_card = -1;
  value_button = -1; // -1
  waitingForBzrAck = false;
  buzzerCount = 0;
  BzrMsgId = 0;
  old_value_buzzer = -1;
  current_value_buzzer = -1;

  last_key = 0;
  attente_retour = 0;
  attente_confirmation = false;
  envoi_effectue = false;
  conveyorToBase = true;
  retour_en_cours = false;
  retour_ideal = false;

  wrong_card = false;
  livraison_faite = false;
  timerAttentePriseDemarre = false;
}

bool toutesCartesVerifiees() {
  return carte1_verifiee && carte2_verifiee && carte3_verifiee && carte4_verifiee;
}


bool checkTimerCard1(int key) {
  return ((key == 1) && ((millis() - tempsDepart) >= t01));
}

bool checkTimerCard2(int key) {
  return ((key == 2) && ((millis() - tempsDepart) >= (t12 + t01)));
}

bool checkTimerCard3(int key) {
  return ((key == 3) && ((millis() - tempsDepart) >= (t23 + t12 + t01)));
}

bool checkTimerCard4(int key) {
  return ((key == -2) && ((millis() - tempsDepart) >= (t34 + t23 + t12 + t01)));
}
