#include "base.h"


char last_key = 0;          
bool attente_confirmation = false;
bool envoi_effectue = false;
bool livraison_faite = false;


byte row_pins[] = {D2, D3, D4, A0}; //row pins of the keypad
byte column_pins[] = {D6, A1, D8, D9}; //column pins of the keypad

//Declaration of the keys of the keypad
char hexaKeys[sizeof(row_pins) / sizeof(byte)][sizeof(column_pins) / sizeof(byte)] =
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'0', 'F', 'E', 'D'}
};

char forbidden_keys[12] = {'A', '4', '5', '6', 'B', '7', '8', '9', 'C', '0', 'F', 'D'};
char bouton_envoi = 'E';

//define object for the keypad
Keypad kypd = Keypad( makeKeymap(hexaKeys), row_pins, column_pins, sizeof(row_pins) / sizeof(byte), sizeof(column_pins) / sizeof(byte)); 
Adafruit_SSD1331 display = Adafruit_SSD1331(CS, DC, MOSI, SCK, RES); 


void afficher_en_cours(char key) {
  display.fillScreen(black);
  display.setTextColor(white);
  display.setCursor(30, 10);
  display.setTextSize(5);
  display.print(key);
  display.setTextColor(cyan);
  display.setCursor(15, 50);
  display.setTextSize(1);
  display.print("EN COURS...");
}

void afficher_envoi(char key) {
  display.fillScreen(black);
  display.setTextColor(green);
  display.setCursor(30, 10);
  display.setTextSize(5);
  display.print(key);
  display.setTextColor(green);
  display.setCursor(15, 50);
  display.setTextSize(1);
  display.print("ENVOYE !");
}

void afficher_interdit() {
  display.fillScreen(black);
  display.setTextColor(red);
  display.setCursor(30, 10);
  display.setTextSize(5);
  display.print("X");
  display.setTextColor(red);
  display.setCursor(15, 50); 
  display.setTextSize(1);
  display.print("PAS POSSIBLE");
}

bool is_forbidden(char key){
for (int i = 0; i < 12; i++) {
  if (key == forbidden_keys[i]) {
    return true;
  }
}
return false;
}

void afficher_setup(){
  display.fillScreen(black);
  display.setTextColor(blue);
  display.setCursor(30, 10);
  display.setTextSize(5);
  display.print("...");
  display.setTextColor(red);
  display.setCursor(15, 50); 
  display.setTextSize(1);
  display.print("ATTENTE");
}
