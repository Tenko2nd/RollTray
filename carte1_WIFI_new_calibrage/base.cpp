#include "base.h"

// [INITIALISATION]

char last_key = 0;

bool attente_confirmation = false;
bool envoi_effectue = false;
bool conveyorToBase = true;
bool retour_en_cours = false;
extern bool retour_ideal = false;
bool wrong_card = false;
bool livraison_faite = false;
bool timerAttentePriseDemarre = false;

unsigned long attente_retour = 0;

const int ATTENTE = 10000;

byte etat_bp_relache = 1;
byte etat_bp_appuye = 0;

affichage_state old_affichage = SETUP;
affichage_state current_affichage = SETUP;

// Pins du Keypad
byte row_pins[] = { ROW1, ROW2, ROW3, ROW4 };
byte column_pins[] = { COL1, COL2, COL3, COL4 };

char hexaKeys[sizeof(row_pins) / sizeof(byte)][sizeof(column_pins) / sizeof(byte)] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '0', 'F', 'E', 'D' }
};

// Keys "interdites" (non utilisées dans le projet)
char forbidden_keys[12] = { 'A', '4', '5', '6', 'B', '7', '8', '9', 'C', '0', 'F', 'D' };
// Bouton d'envoi d'une commande
char bouton_envoi = 'E';
char bouton_calibrage = 'C';

//define object for the keypad
Keypad kypd = Keypad(makeKeymap(hexaKeys), row_pins, column_pins, sizeof(row_pins) / sizeof(byte), sizeof(column_pins) / sizeof(byte));
Adafruit_SSD1331 display = Adafruit_SSD1331(CS, DC, MOSI, SCK, RES);


// [PARTIE ECRAN OLED]

// Initialisation de l'écran
void afficher_setup() {
  clear_screen(YELLOW);

  display.fillScreen(YELLOW);
  display.setTextColor(BLACK);
  display.setTextSize(2);
  String setup_msg = "HELLO!";
  int16_t x_setup = (display.width() - (setup_msg.length() * 6 * 2)) / 2;
  int16_t y_setup = (display.height() / 2) - (2 * 8) - 2;
  display.setCursor(x_setup, y_setup);
  display.print(setup_msg);

  display.setTextColor(BLACK);
  display.setTextSize(1);
  String wait_msg = "VOTRE CHOIX ?";
  int16_t x_wait = (display.width() - (wait_msg.length() * 6 * 1)) / 2;
  int16_t y_wait = (display.height() / 2) + 2;
  display.setCursor(x_wait, y_wait);
  display.print(wait_msg);
}

// Ecran lors du choix de la touche (1, 2 ou 3) avant envoi
void afficher_en_cours(char key) {
  clear_screen(YELLOW);
  display.fillScreen(YELLOW);

  display.setTextColor(BLACK);
  display.setTextSize(2);
  int16_t x_key = (display.width() / 2) - 30;
  int16_t y_key = (display.height() / 2) - (4 * 8 / 2) - 5;
  display.setCursor(x_key, y_key);
  display.print("CHOIX");

  display.setTextColor(BLACK);
  display.setTextSize(2);
  int16_t x_status = (display.width() / 2);
  int16_t y_status = y_key + (4 * 8);
  if (y_status > display.height() - 8) y_status = display.height() - 8;
  display.setCursor(x_status, y_status);
  display.print(key);
}

// Ecran après envoi (après bouton "E")
void afficher_envoi(char key) {

  clear_screen(YELLOW);

  display.fillScreen(YELLOW);

  display.setTextColor(BLACK);
  display.setTextSize(2);
  int16_t x_key = display.width() / 2;
  int16_t y_key = (display.height() / 2) - (4 * 8 / 2) - 5;
  display.setCursor(x_key, y_key);
  display.print(key);

  display.setTextColor(GREEN);
  display.setTextSize(2);
  int16_t x_status = 10;
  int16_t y_status = y_key + (4 * 8);
  if (y_status > display.height() - (2 * 8)) y_status = display.height() - (2 * 8);
  display.setCursor(x_status, y_status);
  display.print("ENVOYE!");
}

// Ecran lorsqu'une touche "interdite" est appuyée
void afficher_interdit() {
  clear_screen(YELLOW);

  display.fillScreen(YELLOW);

  display.setTextColor(RED);
  display.setTextSize(2);
  int16_t x_symbol = (display.width() / 2) - 30;
  int16_t y_symbol = (display.height() / 2) - (4 * 8 / 2) - 5;
  display.setCursor(x_symbol, y_symbol);
  display.print("CHOIX");

  display.setTextColor(RED);
  display.setTextSize(1);
  int16_t x_status = (display.width() - (12 * 6 * 1)) / 2;
  int16_t y_status = (display.height() / 2) - (4 * 8 / 2) + 15;

  display.setCursor(x_status, y_status);
  display.print("IMPOSSIBLE !");
}

// Est-ce que la touche est interdite ?
bool is_forbidden(char key) {
  for (int i = 0; i < 12; i++) {
    if (key == forbidden_keys[i]) {
      return true;
    }
  }
  return false;
}

// Ecran lorsque le convoyeur revient
void afficher_retour() {
  clear_screen(MAGENTA);

  display.fillScreen(MAGENTA);

  display.setTextColor(WHITE);
  display.setTextSize(2);
  String return_msg = "RETOUR";
  int16_t x_return = (display.width() - (return_msg.length() * 6 * 2)) / 2;
  int16_t y_return = (display.height() / 2) - (2 * 8 / 2);
  display.setCursor(x_return, y_return);
  display.print(return_msg);

  display.setTextSize(2);
  display.setCursor(display.width() / 2, (display.height() / 2) - (2 * 8 / 2) + 15);
  display.print("...");
}

// Ecran lorsque le convoyeur revient après TIMEOUT
void afficher_retour_timeout() {
  clear_screen(GREEN);

  display.fillScreen(GREEN);

  display.setTextColor(WHITE);
  display.setTextSize(2);
  String return_msg = "RETOUR";
  int16_t x_return = (display.width() - (return_msg.length() * 6 * 2)) / 2;
  int16_t y_return = (display.height() / 2) - (2 * 8 / 2);
  display.setCursor(x_return, y_return);
  display.print(return_msg);

  display.setTextSize(1);
  display.setCursor(display.width() / 2, (display.height() / 2) - (2 * 8 / 2) + 15);
  display.print("TIMEOUT");
}

// Ecran lorsque le convoyeur revient si aucune détection de carte
void afficher_retour_nodetection() {
  clear_screen(BLUE);

  display.fillScreen(BLUE);

  display.setTextColor(WHITE);
  display.setTextSize(2);
  String return_msg = "RETOUR";
  int16_t x_return = (display.width() - (return_msg.length() * 6 * 2)) / 2;
  int16_t y_return = (display.height() / 2) - (2 * 8 / 2);
  display.setCursor(x_return, y_return);
  display.print(return_msg);

  display.setTextSize(1);
  display.setCursor(0, (display.height() / 2) - (2 * 8 / 2) + 20);
  display.print("AUCUNE DETECTION");
}

// Ecran lorsque le plateau est arrivé à la base ou qu'il est arrivé à bonne destination
void afficher_plateau_arrive() {
  clear_screen(MAGENTA);

  display.fillScreen(MAGENTA);

  display.setTextColor(WHITE);
  display.setTextSize(2);
  int16_t x_symbol = 5;
  int16_t y_symbol = (display.height() / 2) - (5 * 8 / 2) - 5;
  display.setCursor(x_symbol, y_symbol);
  display.print("PLATEAU");


  display.setTextColor(WHITE);
  display.setTextSize(2);
  int16_t x_status = 5;
  int16_t y_status = y_symbol + 20;
  if (y_status > display.height() - 8) y_status = display.height() - 8;
  display.setCursor(x_status, y_status);
  display.print("ARRIVE!");
}

// Ecran lors d'une ERREUR
void afficher_erreur() {
  clear_screen(RED);

  display.fillScreen(RED);

  display.setTextColor(WHITE);
  display.setTextSize(2);
  int16_t x_symbol = (display.width() / 2) - 40;
  int16_t y_symbol = (display.height() / 2) - (5 * 8 / 2) - 5;
  display.setCursor(x_symbol, y_symbol);
  display.print("ERREUR!");
}

// Ecran retour après ERREUR
void afficher_retour_erreur() {
  clear_screen(RED);

  display.fillScreen(RED);

  display.setTextColor(WHITE);
  display.setTextSize(2);
  String return_msg = "RETOUR";
  int16_t x_return = (display.width() / 2) - 40;
  int16_t y_return = (display.height() / 2) - (2 * 8 / 2);
  display.setCursor(x_return, y_return);
  display.print(return_msg);

  display.setTextSize(2);
  display.setCursor(display.width() / 2, (display.height() / 2) - (2 * 8 / 2) + 15);
  display.print("...");
}


// Ecran MAJ à chaque passage d'une carte
void afficher_carte_convoyeur(int value_card) {
  clear_screen(MAGENTA);

  display.fillScreen(MAGENTA);

  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor((display.width() / 2) - 40, (display.height() / 2) - 20);
  display.print("CARTE ");
  if (value_card == -1) {
    display.print("0");
  } else {
    display.print(value_card);
  }
  display.setTextSize(1);
  display.setCursor((display.width() / 2) - 40, (display.height() / 2) + 5);
  display.print("EN COURS DE");
  display.setCursor((display.width() / 2) - 40, (display.height() / 2) + 15);
  display.print("LIVRAISON...");
}


// Ecran affichage carte lue
void affichage_calibrage(int value_card) {
  clear_screen(CYAN);

  display.fillScreen(CYAN);

  display.setTextColor(BLACK);
  display.setTextSize(2);
  display.setCursor((display.width() / 2) - 40, (display.height() / 2) - 20);
  display.print("CARTE ");
  if (value_card == -1) {
    display.print("0");
  } else {
    display.print(value_card);
  }
  display.setTextSize(1);
  display.setCursor((display.width() / 2) - 40, (display.height() / 2) + 5);
  display.print("CALIBRÉE!");
}

// Ecran affichage DEBUT calibrage
void affichage_calibrage_debut() {
  clear_screen(CYAN);

  display.fillScreen(CYAN);

  display.setTextColor(BLACK);
  display.setTextSize(2);
  display.setCursor((display.width() / 2) - 40, (display.height() / 2) - 20);
  display.print("DEBUT");
  display.setTextSize(1);
  display.setCursor((display.width() / 2) - 50, (display.height() / 2) + 5);
  display.print("CALIBRAGE (C)");
}

// Ecran affichage FIN de calibrage
void affichage_calibrage_terminee() {
  clear_screen(CYAN);

  display.fillScreen(CYAN);

  display.setTextColor(BLACK);
  display.setTextSize(2);
  display.setCursor((display.width() / 2) - 40, (display.height() / 2) - 20);
  display.print("FIN");
  display.setTextSize(1);
  display.setCursor((display.width() / 2) - 40, (display.height() / 2) + 5);
  display.print("CALIBRAGE");
}


void affichage_setup_wifi() {
  clear_screen(CYAN);

  display.fillScreen(CYAN);

  display.setTextColor(BLACK);
  display.setTextSize(2);
  display.setCursor((display.width() / 2) - 40, (display.height() / 2) - 20);
  display.print("CONNEXION");
  display.setTextSize(2);
  display.setCursor((display.width() / 2) - 40, (display.height() / 2) + 5);
  display.print("WIFI...");
}

void affichage_erreur_wifi() {
  clear_screen(RED);

  display.fillScreen(RED);

  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor((display.width() / 2) - 40, (display.height() / 2) - 20);
  display.print("ERREUR");
  display.setTextSize(1);
  display.setCursor((display.width() / 2) - 40, (display.height() / 2) + 5);
  display.print("CONNEXION WIFI");
}

void affichage_setup_mqtt() {
  clear_screen(CYAN);

  display.fillScreen(CYAN);

  display.setTextColor(BLACK);
  display.setTextSize(2);
  display.setCursor((display.width() / 2) - 40, (display.height() / 2) - 20);
  display.print("CONNEXION");
  display.setTextSize(2);
  display.setCursor((display.width() / 2) - 40, (display.height() / 2) + 5);
  display.print("MQTT...");
}

void affichage_erreur_mqtt() {
  clear_screen(RED);

  display.fillScreen(RED);

  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor((display.width() / 2) - 40, (display.height() / 2) - 20);
  display.print("ERREUR");
  display.setTextSize(1);
  display.setCursor((display.width() / 2) - 40, (display.height() / 2) + 5);
  display.print("CONNEXION MQTT");
}


// Effacer l'écran
void clear_screen(unsigned int couleur) {
  display.fillScreen(couleur);
}

// Switch pour affichage  carte
void choose_current_affichage_card(int value_card) {
  switch (value_card) {
    case 1:
      current_affichage = CARTE1;
      break;

    case 2:
      current_affichage = CARTE2;
      break;

    case 3:
      current_affichage = CARTE3;
      break;

    default:
      break;
  }
}
