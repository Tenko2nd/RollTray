#include <Arduino.h>
#include <Keypad.h>
#include <Adafruit_SSD1331.h>

// Pins du Keypad
#define ROW1 D2
#define ROW2 D3
#define ROW3 D4
#define ROW4 A0
#define COL1 D6
#define COL2 A1
#define COL3 D8
#define COL4 D9

// Pins écran OLED
#define SCK D12
#define MOSI D11
#define CS D10
#define DC D7
#define RES D5

// Pins LEDs
#define ledVert A6
#define ledRouge A7

// Couleurs
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF


enum affichage_state {
  SETUP,
  EN_COURS,
  ENVOI,
  INTERDIT,
  RETOUR,
  PLATEAU_ARRIVE,
  ERREUR,
  RETOUR_ERREUR,
  CARTE1,
  CARTE2,
  CARTE3  
};

extern char last_key;

extern bool attente_confirmation;
extern bool envoi_effectue;
extern bool livraison_faite;
extern bool conveyorToBase;
extern bool retour_en_cours;
extern bool retour_ideal;
extern bool wrong_card;
extern bool timerAttentePriseDemarre;

extern byte row_pins[];
extern byte column_pins[];
extern char hexaKeys[4][4];
extern Keypad kypd;
extern Adafruit_SSD1331 display;

extern byte etat_bp_relache;
extern byte etat_bp_appuye;

extern char forbidden_keys[12];
extern char bouton_envoi;
extern char bouton_calibrage;

extern affichage_state old_affichage;
extern affichage_state current_affichage;

extern unsigned long attente_retour;
extern const int ATTENTE;



void afficher_en_cours(char key);
void afficher_envoi(char key);
void afficher_interdit();
void afficher_setup();
bool is_forbidden(char key);
void clear_screen(unsigned int couleur);
void afficher_retour();
void afficher_retour_timeout();
void afficher_retour_nodetection();
void afficher_plateau_arrive();
void afficher_erreur();
void afficher_retour_erreur();
void afficher_carte_convoyeur(int value_card);
void affichage_calibrage(int value_card);
void choose_current_affichage_card(int value_card);

void affichage_setup_wifi();
void affichage_erreur_wifi();

void affichage_setup_mqtt();
void affichage_erreur_mqtt();

void affichage_calibrage_debut();
void affichage_calibrage_terminee();

