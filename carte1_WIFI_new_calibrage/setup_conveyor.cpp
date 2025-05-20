#include "setup_conveyor.h"

bool carte1_verifiee = false;
bool carte2_verifiee = false;
bool carte3_verifiee = false;
bool carte4_verifiee = false;

bool calibrageFait = false;

void setup_conveyor() {

  moveForward(1);
  delay(100);

  digitalWrite(ledVert, HIGH);
  digitalWrite(ledRouge, HIGH);

  if (!carte1_verifiee && value_card == 1) {
    carte1_verifiee = true;
    affichage_calibrage(value_card);

  }

  else if (!carte2_verifiee && value_card == 2) {
    carte2_verifiee = true;
    affichage_calibrage(value_card);

  }

  else if (!carte3_verifiee && value_card == 3) {
    carte3_verifiee = true;
    affichage_calibrage(value_card);

  }

  else if (!carte4_verifiee && value_card == -2) {
    carte4_verifiee = true;
    affichage_calibrage(value_card);
  }



  if (carte1_verifiee && carte2_verifiee && carte3_verifiee && carte4_verifiee) {
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
    }
  }
}


void reinitialize_variables() {

  digitalWrite(ledVert, LOW);
  digitalWrite(ledRouge, LOW);

  afficher_setup();



  value_card = -1;
  value_button = -1;
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
