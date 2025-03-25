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
  COL2      to               7           to       -
  COL1      to               6           to       -
  ROW4    to               5           to       -
  ROW3    to               4           to       -
  ROW2    to               3           to       -
  ROW1    to               2           to       -

************************************************************************/

#include <Keypad.h>


byte row_pins[] = {D2, D3, D4, D5}; //row pins of the keypad
byte column_pins[] = {D6, D7, D8, D9}; //column pins of the keypad

//Declaration of the keys of the keypad
char hexaKeys[sizeof(row_pins) / sizeof(byte)][sizeof(column_pins) / sizeof(byte)] =
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'0', 'F', 'E', 'D'}
};

Keypad kypd = Keypad( makeKeymap(hexaKeys), row_pins, column_pins, sizeof(row_pins) / sizeof(byte), sizeof(column_pins) / sizeof(byte)); //define object for the keypad

void setup() {
  Serial.begin(9600);
}

void loop()
{
  char current_key = kypd.getKey();  //get keypad state
  if (current_key != 0) { //if a key is pressed
    Serial.println(current_key);
  }
}