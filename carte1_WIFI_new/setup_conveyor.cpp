#include "setup_conveyor.h"


void reinitialize_variables() {

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
