//pin definitions
#define SCK  D12 //serial clock
#define MOSI D11 //master-out slave-in
#define CS   D10 //chip select
#define DC   D7  //data/command control
#define RES  D5  //power reset

#define BP D3 //bouton poussoir


#include <Adafruit_SSD1331.h>


//define colors
#define black   0x0000
#define blue    0x001F
#define red     0xF800
#define green   0x07E0
#define cyan    0x07FF
#define magenta 0xF81F
#define yellow  0xFFE0
#define white   0xFFFF

int etat = 0;


Adafruit_SSD1331 display = Adafruit_SSD1331(CS, DC, MOSI, SCK, RES);


void setup() {
  display.begin();
  pinMode(BP, INPUT);
}


void loop() {
  etat = digitalRead(BP);

  display.fillScreen(black); 

  if (etat == HIGH) { 
    display.setTextColor(red); 
    display.setCursor(15, 15); 
    display.setTextSize(2);
    display.print("Erreur");
    delay(500);
  } else {
    char test[] = {"..."}; //store characters
    display.setTextColor(cyan); //set text color
    display.setCursor(25, 15); //set cursor position (x, y)
    display.setTextSize(2); //set the size of text
    for (int i = 0; i < 4; i++) {
      display.print(test[i]);
      delay(500);
    }
  }

  delay(100); 
}