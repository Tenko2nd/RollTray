/************************************************************************

KeyPad Code

*************************************************************************

  Wiring
  KYPD<----------> Arduino <--------> CLS
  VCC        to                5V        to      VCC
  GND       to               GND      to      GND
  -              to               13         to      RX
  COL4      to               9           to       -
  COL3      to               8           to       -
  COL2      to               A1           to       -
  COL1      to               6           to       -
  ROW4    to               A0           to       -
  ROW3    to               4           to       -
  ROW2    to               3           to       -
  ROW1    to               2           to       -

************************************************************************/

#include <Keypad.h>
#include <Adafruit_SSD1331.h> 

//pin definitions
#define SCK  D12 //serial clock
#define MOSI D11 //master-out slave-in
#define CS   D10 //chip select
#define DC   D7  //data/command control
#define RES  D5  //power reset


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

Keypad kypd = Keypad( makeKeymap(hexaKeys), row_pins, column_pins, sizeof(row_pins) / sizeof(byte), sizeof(column_pins) / sizeof(byte)); //define object for the keypad

//define colors
#define black   0x0000
#define blue    0x001F
#define red     0xF800
#define green   0x07E0
#define cyan    0x07FF
#define magenta 0xF81F
#define yellow  0xFFE0
#define white   0xFFFF


Adafruit_SSD1331 display = Adafruit_SSD1331(CS, DC, MOSI, SCK, RES); 



void setup() {
  Serial.begin(9600);
  display.begin();  

}

void loop()
{
  char current_key = kypd.getKey();  //get keypad state
  if (current_key != 0) { //if a key is pressed
    Serial.println(current_key);
    display.fillScreen(black);  //set background and clear everything
    display.setTextColor(white); //set text color
    display.setCursor(30, 10); //set cursor position (x, y)
    display.setTextSize(5); //set the size of text
    display.print(current_key); // display text
  }

  display.setTextColor(cyan); //set text color
  display.setCursor(15, 50); //set cursor position (x, y)
  display.setTextSize(1); //set the size of text
  display.print("EN COURS...");


}