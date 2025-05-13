#include <Arduino.h>


/************************************************************************

KeyPad + OLED + conveyor

*************************************************************************

  Wiring
  KYPD<----------> Arduino 
  VCC        to                5V      
  GND       to               GND   
  COL4      to               9         
  COL3      to               8      
  COL2      to               A1        
  COL1      to               6    
  ROW4    to               A0  
  ROW3    to               4  
  ROW2    to               3     
  ROW1    to               2   

************************************************************************/


#include <Keypad.h>
#include <Adafruit_SSD1331.h> 

//pin definitions : pour l'écran
#define SCK  D12 //serial clock
#define MOSI D11 //master-out slave-in
#define CS   D10 //chip select
#define DC   D7  //data/command control
#define RES  D5  //power reset

//define colors
#define black   0x0000
#define blue    0x001F
#define red     0xF800
#define green   0x07E0
#define cyan    0x07FF
#define magenta 0xF81F
#define yellow  0xFFE0
#define white   0xFFFF

extern char last_key;          
extern bool attente_confirmation;
extern bool envoi_effectue;
extern bool livraison_faite;


extern byte row_pins[]; //row pins of the keypad
extern byte column_pins[]; //column pins of the keypad

//Declaration of the keys of the keypad
extern char hexaKeys[4][4];

extern char forbidden_keys[12];
extern char bouton_envoi;

//define object for the keypad
extern Keypad kypd; 
extern Adafruit_SSD1331 display; 

void afficher_en_cours(char key);
void afficher_envoi(char key);
void afficher_interdit();
void afficher_setup();
bool is_forbidden(char key);


